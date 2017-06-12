/**
 * collectd - src/vm_disk.c
 * Copyright (C) 2017-2022  Red Hat Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; only version 2 of the license is applicable.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Authors:
 *   dengshijun <dengshijun1992@gmail.com>
 **/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h> /* For O_* constants */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <malloc.h>
#include <math.h>
#include <unistd.h>

#include <unistd.h>
#include <error.h>
#include <stdint.h>

#include "collectd.h"
#include "common.h"
#include "plugin.h"
#include "utils_complain.h"
#include "utils_ignorelist.h"

#include <libgen.h> /* for basename(3) */
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#include <guestfs.h>/*libguestfs*/

/* Plugin name */
#define PLUGIN_NAME "vm_disk"

#define STRNEQ(a, b) (strcmp((a),(b)) != 0)

//每台虚拟机拥有的device的最大数量
#define MAX_DEVNUM_PER_VM 64
//存储虚拟机device信息的共享存储区域的大小
#define MAX_VM_DISK_INFO_SHM_NUM 32
//domain名的最大长度
#define MAX_DOMAIN_NAME_LEN DATA_MAX_NAME_LEN
//device名称的最大长度
#define MAX_DEV_NAME_LEN 64

/* HostnameFormat. */
#define HF_MAX_FIELDS 3

/* ERROR(...) macro for vm_diskerrors. */
#define VIRT_ERROR(conn, s)                                                    \
		do {                                                                         \
				virErrorPtr err;                                                           \
				err = (conn) ? virConnGetLastError((conn)) : virGetLastError();            \
				if (err)                                                                   \
				ERROR("%s: %s", (s), err->message);                                      \
		} while (0)

static const char *config_keys[] = { "Connection", "RefreshInterval",
		"IgnoreSelected", "HostnameFormat", "PluginInstanceFormat",
		NULL };
#define NR_CONFIG_KEYS ((sizeof config_keys / sizeof config_keys[0]) - 1)
/* Connection. */
static virConnectPtr conn = 0;
static char *conn_string = NULL;
static c_complain_t conn_complain = C_COMPLAIN_INIT_STATIC;

/* Seconds between list refreshes, 0 disables completely. */
static int interval = 30;

/* List of domains, if specified. */
static ignorelist_t *il_domains = NULL;

/* Actual list of domains found on last refresh. */

//domains的名称和数量
static char (*domains)[MAX_DOMAIN_NAME_LEN] = NULL;
static int nr_domains = 0;

//libguestfs信息采集循环流程控制
static int collect_loop = 1;

/*虚拟机设备总容量/可用容量信息*/
typedef struct vm_device_info_shm {
	int dev_num; //设备的数量
	int64_t device_usage_info[MAX_DEVNUM_PER_VM][2]; //0 total 1 free
	char dev_name[MAX_DEVNUM_PER_VM][MAX_DEV_NAME_LEN]; //设备的名称
	char domain_name[MAX_DOMAIN_NAME_LEN]; //虚拟机的名称
}*vm_device_info_shm_ptr;

static int vm_device_info_shmid;
static struct vm_device_info_shm *vm_device_info_shmaddr;
//libguestfs存储读取信息后存储到vm_device_info_shmaddr中的轮询下标
static int mem_idx = 0;
//进程同步
static sem_t *sem_pv; /*定义Posix有名信号灯*/
//进程控制相关
const char * PV_NAME = "sem_pv";

static void free_domains(void);
static int add_domain(const char *);
static int refresh_lists(void);
static int vm_device_info_shmget(void);
static void vm_device_info_shm_init(void);

enum hf_field {
	hf_none = 0, hf_hostname, hf_name, hf_uuid
};

static enum hf_field hostname_format[HF_MAX_FIELDS] = { hf_name };

/* PluginInstanceFormat */
#define PLGINST_MAX_FIELDS 2
enum plginst_field {
	plginst_none = 0, plginst_name, plginst_uuid
};

static enum plginst_field plugin_instance_format[PLGINST_MAX_FIELDS] = {
		plginst_none };

/* Time that we last refreshed. */
static time_t last_refresh = (time_t) 0;

/*调用libguestfs获取名称为domains[dom_no]的虚拟机的device的信息*/
int update_disk_usage(int dom_no) {
	char **devices = NULL;
	char **fses = NULL;
	char* dom = domains[dom_no];
	guestfs_h *g;

	g = guestfs_create();

	if (g == NULL) {
		perror("vm_disk:failed to create libguestfs handle");
		exit(EXIT_FAILURE);
	}

	guestfs_add_domain(g, (const char *) dom, GUESTFS_ADD_DOMAIN_READONLY, 1,
			-1);
	guestfs_set_identifier(g, (const char *) dom);
	guestfs_set_trace(g, 0);
	guestfs_set_verbose(g, 0);

	if (guestfs_launch(g) == -1) {
		perror("vm_disk:guestfs_launch\n");
		return -1;
	}

	devices = guestfs_list_devices(g);

	if (devices == NULL) {
		perror("vm_disk:guesfs_list_devices(g) == NULL");
		return -1;
	}

	fses = guestfs_list_filesystems(g);

	if (fses == NULL) {
		perror("vm_disk:guestfs_list_filesystem(g)");
		return -1;
	}

	int count = 0;
	sem_wait(sem_pv);

	for (int i = 0; fses[i] != NULL && count < MAX_VM_DISK_INFO_SHM_NUM; i +=
			2) {
		if (STRNEQ(fses[i + 1],
				"") && STRNEQ(fses[i + 1], "swap") && STRNEQ(fses[i + 1], "unknown")) {
			char *dev = fses[i];

			guestfs_push_error_handler(g, NULL, NULL);

			if (guestfs_mount_ro(g, dev, "/") == 0) {
				struct guestfs_statvfs * stat = guestfs_statvfs(g, "/");
				strncpy((vm_device_info_shmaddr[mem_idx].dev_name)[count], dev,
						MAX_DEV_NAME_LEN);
				(vm_device_info_shmaddr[mem_idx].device_usage_info)[count][0] =
						(derive_t) stat->blocks;
				(vm_device_info_shmaddr[mem_idx].device_usage_info)[count][1] =
						(derive_t) stat->bfree;
				fprintf(stderr,
						"vm_disk:submit data to share memory,values=[mem_idx=%d,vm=%s,dev=%s,total=%ld,free=%ld]\n",
						mem_idx, dom, dev,
						(vm_device_info_shmaddr[mem_idx].device_usage_info)[count][0],
						(vm_device_info_shmaddr[mem_idx].device_usage_info)[count][1]);
				count++;
				sfree(stat);
			} else {
				continue;
			}
		}
	}

	strncpy(vm_device_info_shmaddr[mem_idx].domain_name, dom,
			MAX_DOMAIN_NAME_LEN);
	vm_device_info_shmaddr[mem_idx].dev_num = count;
	mem_idx++;
	mem_idx %= MAX_VM_DISK_INFO_SHM_NUM;
	sem_post(sem_pv);
	guestfs_close(g);
	sfree(devices);
	sfree(fses);
	return 0;
}

/*轮询获取在domains列表中虚拟机的device的信息*/
int refresh_vm_disk_usage0() {
	time_t t;

	if (conn == NULL) {
		/* `conn_string == NULL' is acceptable. */
		conn = virConnectOpenReadOnly(conn_string);

		if (conn == NULL) {
			c_complain(LOG_ERR, &conn_complain,
			PLUGIN_NAME " plugin: Unable to connect: "
			"virConnectOpenReadOnly failed.");
			return -1;
		}
	}

	c_release(LOG_NOTICE, &conn_complain,
	PLUGIN_NAME " plugin: Connection established.");

	time(&t);

	/* Need to refresh domain or device lists? */
	if ((last_refresh == (time_t) 0)
			|| ((interval > 0) && ((last_refresh + interval) <= t))) {
		if (refresh_lists() != 0) {
			if (conn != NULL)
				virConnectClose(conn);

			conn = NULL;
			return -1;
		}

		last_refresh = t;
	}

	for (int i = 0; i < nr_domains; ++i) {
		update_disk_usage(i);
	}

	return 0;
}

/*循环刷新*/
static void refresh_vm_disk_usage() {
	int seep_time = interval;
	int unit_time = 5; //获取一台虚拟机相关信息所耗费的时间:默认为5s
	while (collect_loop) {
		refresh_vm_disk_usage0();
		seep_time = interval - unit_time * nr_domains;
		if (seep_time > 0) {
			sleep(seep_time); //单位是秒
		}
	}
}

/*采集流程:先刷新虚拟机列表存放到domains,根据domains中的数据进行一趟轮询查询*/
static int start_collect_work() {
	int pid = 0;
	vm_device_info_shmget();

	sem_pv = sem_open(PV_NAME, O_CREAT, 0644, 1);
	pid = fork();

	if (pid == 0) {
		vm_device_info_shmaddr = (struct vm_device_info_shm *) shmat(
				vm_device_info_shmid, NULL, 0);
		vm_device_info_shm_init();

		if (vm_device_info_shmaddr == (void*) (-1)) {
			perror("vm_disk:shmat addr error!\n");
			return -1;
		}

		fprintf(stderr,
				"vm_disk:process collects data from virtual machines pid=%d  ppid=%d\n",
				getpid(), getppid());
		refresh_vm_disk_usage();
	} else if (pid > 0) {
		vm_device_info_shmaddr = (struct vm_device_info_shm*) shmat(
				vm_device_info_shmid, NULL, 0);
		vm_device_info_shm_init();

		if (vm_device_info_shmaddr == (void*) (-1)) {
			perror("vm_disk:shmat addr error!\n");
			return -1;
		}

		fprintf(stderr,
				"vm_disk:process submits data to collectd pid=%d  ppid=%d\n",
				getpid(), getppid());
	} else {
		fprintf(stderr, "vm_disk:fork error\n");
		return -1;
	}

	return 0;
}

static int vir_init(void) {
	if (virInitialize() != 0)
		return -1;

	return 0;
}
/*共享内存申请*/
static int vm_device_info_shmget(void) {
	vm_device_info_shmid = shmget(IPC_PRIVATE,
			sizeof(struct vm_device_info_shm) * MAX_VM_DISK_INFO_SHM_NUM,
			IPC_CREAT | 0600);

	if (vm_device_info_shmid < 0) {
		perror("vm_disk:get shm id error!\n");
		exit(1);
	}

	return 0;
}

/*初始化存储虚拟机device信息的存储区域*/
static void vm_device_info_shm_init() {

	for (int i = 0; i < MAX_VM_DISK_INFO_SHM_NUM; i++) {
		for (int j = 0; j < MAX_DEVNUM_PER_VM; j++) {
			(vm_device_info_shmaddr[i].device_usage_info)[j][0] = -1;
			(vm_device_info_shmaddr[i].device_usage_info)[j][1] = -1;
			strcpy(vm_device_info_shmaddr[i].dev_name[j], "");
		}

		strcpy(vm_device_info_shmaddr[i].domain_name, "");
		vm_device_info_shmaddr[i].dev_num = 0;
	}

	mem_idx = 0;
}

static int vm_disk_config(const char *key, const char *value) {
	if (virInitialize() != 0)
		return 1;

	if (il_domains == NULL)
		il_domains = ignorelist_create(1);

	if (strcasecmp(key, "Connection") == 0) {
		char *tmp = strdup(value);

		if (tmp == NULL) {
			ERROR(PLUGIN_NAME " plugin: Connection strdup failed.");
			return 1;
		}

		sfree(conn_string);
		conn_string = tmp;
		return 0;
	}

	if (strcasecmp(key, "RefreshInterval") == 0) {
		char *eptr = NULL;
		interval = strtol(value, &eptr, 10);

		if (eptr == NULL || *eptr != '\0')
			return 1;

		return 0;
	}

	if (strcasecmp(key, "Domain") == 0) {
		if (ignorelist_add(il_domains, value))
			return 1;

		return 0;
	}

	if (strcasecmp(key, "HostnameFormat") == 0) {
		char *value_copy;
		char *fields[HF_MAX_FIELDS];
		int n;

		value_copy = strdup(value);

		if (value_copy == NULL) {
			ERROR(PLUGIN_NAME " plugin: strdup failed.");
			return -1;
		}

		n = strsplit(value_copy, fields, HF_MAX_FIELDS);

		if (n < 1) {
			sfree(value_copy);
			ERROR(PLUGIN_NAME " plugin: HostnameFormat: no fields");
			return -1;
		}

		for (int i = 0; i < n; ++i) {
			if (strcasecmp(fields[i], "hostname") == 0)
				hostname_format[i] = hf_hostname;
			else if (strcasecmp(fields[i], "name") == 0)
				hostname_format[i] = hf_name;
			else if (strcasecmp(fields[i], "uuid") == 0)
				hostname_format[i] = hf_uuid;
			else {
				ERROR(PLUGIN_NAME " plugin: unknown HostnameFormat field: %s",
						fields[i]);
				sfree(value_copy);
				return -1;
			}
		}

		sfree(value_copy);

		for (int i = n; i < HF_MAX_FIELDS; ++i)
			hostname_format[i] = hf_none;

		return 0;
	}

	if (strcasecmp(key, "PluginInstanceFormat") == 0) {
		char *value_copy;
		char *fields[PLGINST_MAX_FIELDS];
		int n;

		value_copy = strdup(value);

		if (value_copy == NULL) {
			ERROR(PLUGIN_NAME " plugin: strdup failed.");
			return -1;
		}

		n = strsplit(value_copy, fields, PLGINST_MAX_FIELDS);

		if (n < 1) {
			sfree(value_copy);
			ERROR(PLUGIN_NAME " plugin: PluginInstanceFormat: no fields");
			return -1;
		}

		for (int i = 0; i < n; ++i) {
			if (strcasecmp(fields[i], "none") == 0) {
				plugin_instance_format[i] = plginst_none;
				break;
			} else if (strcasecmp(fields[i], "name") == 0)
				plugin_instance_format[i] = plginst_name;
			else if (strcasecmp(fields[i], "uuid") == 0)
				plugin_instance_format[i] = plginst_uuid;
			else {
				ERROR(
						PLUGIN_NAME " plugin: unknown PluginInstanceFormat field: %s",
						fields[i]);
				sfree(value_copy);
				return -1;
			}
		}

		sfree(value_copy);

		for (int i = n; i < PLGINST_MAX_FIELDS; ++i)
			plugin_instance_format[i] = plginst_none;

		return 0;
	}

	/* Unrecognised option. */
	return -1;
}

static int refresh_lists(void) {
	int n;

	n = virConnectNumOfDomains(conn);

	if (n < 0) {
		VIRT_ERROR(conn, "reading number of domains");
		return -1;
	}

	if (n > 0) {
		int *domids;

		/* Get list of domains. */
		domids = malloc(sizeof(*domids) * n);

		if (domids == NULL) {
			ERROR(PLUGIN_NAME " plugin: malloc failed.");
			return -1;
		}

		n = virConnectListDomains(conn, domids, n);

		if (n < 0) {
			VIRT_ERROR(conn, "reading list of domains");
			sfree(domids);
			return -1;
		}

		free_domains();

		/* Fetch each domain and add it to the list, unless ignore. */
		for (int i = 0; i < n; ++i) {
			virDomainPtr dom = NULL;
			const char *name;
			char *xml = NULL;
			xmlDocPtr xml_doc = NULL;
			xmlXPathContextPtr xpath_ctx = NULL;
			xmlXPathObjectPtr xpath_obj = NULL;

			dom = virDomainLookupByID(conn, domids[i]);

			if (dom == NULL) {
				VIRT_ERROR(conn, "virDomainLookupByID");
				/* Could be that the domain went away -- ignore it anyway. */
				continue;
			}

			name = virDomainGetName(dom);

			if (name == NULL) {
				VIRT_ERROR(conn, "virDomainGetName");
				goto cont;
			}

			if (il_domains && ignorelist_match(il_domains, name) != 0)
				goto cont;

			if (add_domain(name) < 0) {
				ERROR(PLUGIN_NAME " plugin: malloc failed.");
				goto cont;
			}

			cont:

			if (xpath_obj)
				xmlXPathFreeObject(xpath_obj);

			if (xpath_ctx)
				xmlXPathFreeContext(xpath_ctx);

			if (xml_doc)
				xmlFreeDoc(xml_doc);

			sfree(xml);
		}

		sfree(domids);
	}

	return 0;
}

static void free_domains(void) {
	sfree(domains);
	domains = NULL;
	nr_domains = 0;
}

static int add_domain(const char * dom) {
	char (*new_ptr)[MAX_DOMAIN_NAME_LEN];
	int new_size = sizeof(char) * MAX_DOMAIN_NAME_LEN * (nr_domains + 1);

	if (domains)
		new_ptr = realloc(domains, new_size);
	else
		new_ptr = malloc(new_size);

	if (new_ptr == NULL)
		return -1;

	domains = new_ptr;
	strncpy(domains[nr_domains], dom,
			MIN(MAX_DOMAIN_NAME_LEN - 1, strlen(dom)));
	domains[nr_domains][MIN(MAX_DOMAIN_NAME_LEN - 1, strlen(dom))] = '\0';
	return nr_domains++;
}

static void init_value_list(value_list_t *vl, virDomainPtr dom) {
	int n;
	const char *name;
	char uuid[VIR_UUID_STRING_BUFLEN];

	sstrncpy(vl->plugin, PLUGIN_NAME, sizeof(vl->plugin));

	vl->host[0] = '\0';

	for (int i = 0; i < HF_MAX_FIELDS; ++i) {
		if (hostname_format[i] == hf_none)
			continue;

		n = DATA_MAX_NAME_LEN - strlen(vl->host) - 2;

		if (i > 0 && n >= 1) {
			strncat(vl->host, ":", 1);
			n--;
		}

		switch (hostname_format[i]) {
		case hf_none:
			break;

		case hf_hostname:
			strncat(vl->host, hostname_g, n);
			break;

		case hf_name:
			name = virDomainGetName(dom);

			if (name)
				strncat(vl->host, name, n);

			break;

		case hf_uuid:
			if (virDomainGetUUIDString(dom, uuid) == 0)
				strncat(vl->host, uuid, n);

			break;
		}
	}

	vl->host[sizeof(vl->host) - 1] = '\0';

	for (int i = 0; i < PLGINST_MAX_FIELDS; ++i) {
		if (plugin_instance_format[i] == plginst_none)
			continue;

		n = sizeof(vl->plugin_instance) - strlen(vl->plugin_instance) - 2;

		if (i > 0 && n >= 1) {
			strncat(vl->plugin_instance, ":", 1);
			n--;
		}

		switch (plugin_instance_format[i]) {
		case plginst_none:
			break;

		case plginst_name:
			name = virDomainGetName(dom);

			if (name)
				strncat(vl->plugin_instance, name, n);

			break;

		case plginst_uuid:
			if (virDomainGetUUIDString(dom, uuid) == 0)
				strncat(vl->plugin_instance, uuid, n);

			break;
		}
	}

	vl->plugin_instance[sizeof(vl->plugin_instance) - 1] = '\0';

}
static void submit(virDomainPtr dom, char const *type,
		char const *type_instance, value_t *values, size_t values_len) {
	value_list_t vl = VALUE_LIST_INIT;
	init_value_list(&vl, dom);

	vl.values = values;
	vl.values_len = values_len;

	sstrncpy(vl.type, type, sizeof(vl.type));

	if (type_instance != NULL)
		sstrncpy(vl.type_instance, type_instance, sizeof(vl.type_instance));

	plugin_dispatch_values(&vl);
}

static void submit_derive2(const char *type, derive_t v0, derive_t v1,
		virDomainPtr dom, const char *devname) {
	value_t values[] = { { .derive = v0 }, { .derive = v1 }, };

	submit(dom, type, devname, values, STATIC_ARRAY_SIZE(values));
}

/*读取数据到collectd*/
static int vm_disk_read() {
	time_t t;

	if (conn == NULL) {
		/* `conn_string == NULL' is acceptable. */
		conn = virConnectOpenReadOnly(conn_string);

		if (conn == NULL) {
			c_complain(LOG_ERR, &conn_complain,
			PLUGIN_NAME " plugin: Unable to connect: "
			"virConnectOpenReadOnly failed.");
			return -1;
		}
	}

	c_release(LOG_NOTICE, &conn_complain,
	PLUGIN_NAME " plugin: Connection established.");

	time(&t);

	/* Need to refresh domain or device lists? */
	if ((last_refresh == (time_t) 0)
			|| ((interval > 0) && ((last_refresh + interval) <= t))) {
		if (refresh_lists() != 0) {
			if (conn != NULL)
				virConnectClose(conn);

			conn = NULL;
			return -1;
		}

		last_refresh = t;
	}

	sem_wait(sem_pv);
	for (int i = 0; i < MAX_VM_DISK_INFO_SHM_NUM; i++) {
		/*检测vm_device_info_shm v是否有效:有效返回1,否则返回0*/
		if (vm_device_info_shmaddr[i].dev_num == 0
				|| strlen(vm_device_info_shmaddr[i].domain_name) == 0) {
			continue;
		}

		virDomainPtr dom = virDomainLookupByName(conn,
				vm_device_info_shmaddr[i].domain_name);

		if (dom == NULL) {
			continue; //这里不上面的语句合并是因为上面可以避免无效的.domain_name
		}

		for (int j = 0; j < vm_device_info_shmaddr[i].dev_num; j++) {
			if ((vm_device_info_shmaddr[i].device_usage_info)[j][0] != -1
					&& (vm_device_info_shmaddr[i].device_usage_info)[j][1] != -1
					&& strlen((vm_device_info_shmaddr[i].dev_name)[j]) != 0) {
				submit_derive2("vm_disk",
						(vm_device_info_shmaddr[i].device_usage_info)[j][0],
						(vm_device_info_shmaddr[i].device_usage_info)[j][1],
						dom, (vm_device_info_shmaddr[i].dev_name)[j]);
				fprintf(stderr,
						"vm_disk:submit data collectd,values=[vm=%s,dev=%s,total=%ld,free=%ld]\n",
						vm_device_info_shmaddr[i].domain_name,
						(vm_device_info_shmaddr[i].dev_name)[j],
						(vm_device_info_shmaddr[i].device_usage_info)[j][0],
						(vm_device_info_shmaddr[i].device_usage_info)[j][1]);
				//提交数据后,清空相关数据区域,避免重复提交数据,后面有类似操作
				(vm_device_info_shmaddr[i].device_usage_info)[j][0] =
						(vm_device_info_shmaddr[i].device_usage_info)[j][1] =
								-1;
			}

			strcpy(vm_device_info_shmaddr[i].dev_name[j], "");
			vm_device_info_shmaddr[i].dev_num = 0;
		}

		strcpy(vm_device_info_shmaddr[i].domain_name, "");
	}

	sem_post(sem_pv);
	return 0;
}

static int vm_disk_shutdown(void) {
	free_domains();
	collect_loop = 0;

	if (conn != NULL)
		virConnectClose(conn);

	conn = NULL;
	ignorelist_free(il_domains);
	il_domains = NULL;
	shmdt(vm_device_info_shmaddr);
	shmctl(vm_device_info_shmid, IPC_RMID, NULL);
	sem_destroy(sem_pv);
	return 0;
}

void module_register(void) {
	plugin_register_config(PLUGIN_NAME, vm_disk_config, config_keys,
			NR_CONFIG_KEYS);
	plugin_register_init(PLUGIN_NAME, vir_init);
	plugin_register_read(PLUGIN_NAME, vm_disk_read);
	fprintf(stderr, "vm_disk:module_register:pid=%d  ppid=%d\n", getpid(),
			getppid());
	start_collect_work();
	plugin_register_shutdown(PLUGIN_NAME, vm_disk_shutdown);
}

