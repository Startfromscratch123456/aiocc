/**
 * collectd - src/virt.c
 * Copyright (C) 2006-2008  Red Hat Inc.
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
 *   Richard W.M. Jones <rjones@redhat.com>
 **/

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

#include "guestfs.h"
/* Plugin name */
#define PLUGIN_NAME "vm_disk"

static const char *config_keys[] = {"Connection",

    "RefreshInterval",

    "Domain",
    "BlockDevice",
    "BlockDeviceFormat",
    "BlockDeviceFormatBasename",
    "InterfaceDevice",
    "IgnoreSelected",

    "HostnameFormat",
    "PluginInstanceFormat",

    NULL};
#define NR_CONFIG_KEYS ((sizeof config_keys / sizeof config_keys[0]) - 1)

/* Connection. */
static virConnectPtr conn = 0;
static char *conn_string = NULL;
static c_complain_t conn_complain = C_COMPLAIN_INIT_STATIC;

/* Seconds between list refreshes, 0 disables completely. */
static int interval = 60;

/* List of domains, if specified. */
static ignorelist_t *il_domains = NULL;
/* List of block devices, if specified. */
static ignorelist_t *il_block_devices = NULL;

static int ignore_device_match(ignorelist_t *, const char *domname,
        const char *devpath);

/* Actual list of domains found on last refresh. */
static virDomainPtr *domains = NULL;
static int nr_domains = 0;

static void free_domains(void);
static int add_domain(virDomainPtr dom);

/* Actual list of block devices found on last refresh. */
struct block_device {
    virDomainPtr dom; /* domain */
    char *path;       /* name of block device */
};

static struct block_device *block_devices = NULL;
static int nr_block_devices = 0;

static void free_block_devices(void);
static int add_block_device(virDomainPtr dom, const char *path);

/* HostnameFormat. */
#define HF_MAX_FIELDS 3

enum hf_field { hf_none = 0, hf_hostname, hf_name, hf_uuid };

static enum hf_field hostname_format[HF_MAX_FIELDS] = {hf_name};

/* PluginInstanceFormat */
#define PLGINST_MAX_FIELDS 2

enum plginst_field { plginst_none = 0, plginst_name, plginst_uuid };

static enum plginst_field plugin_instance_format[PLGINST_MAX_FIELDS] = {
    plginst_none};

/* BlockDeviceFormat */
enum bd_field { target, source };

/* BlockDeviceFormatBasename */
_Bool blockdevice_format_basename = 0;
static enum bd_field blockdevice_format = target;

/* Time that we last refreshed. */
static time_t last_refresh = (time_t)0;

static int refresh_lists(void);

/* ERROR(...) macro for vm_diskerrors. */
#define VIRT_ERROR(conn, s)                                                    \
    do {                                                                         \
        virErrorPtr err;                                                           \
        err = (conn) ? virConnGetLastError((conn)) : virGetLastError();            \
        if (err)                                                                   \
        ERROR("%s: %s", (s), err->message);                                      \
    } while (0)

static void init_value_list(value_list_t *vl, virDomainPtr dom) {
    int n;
    const char *name;
    char uuid[VIR_UUID_STRING_BUFLEN];

    sstrncpy(vl->plugin, PLUGIN_NAME, sizeof(vl->plugin));

    vl->host[0] = '\0';

    /* Construct the hostname field according to HostnameFormat. */
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

    /* Construct the plugin instance field according to PluginInstanceFormat. */
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

} /* void init_value_list */

static void submit(virDomainPtr dom, char const *type,
        char const *type_instance, value_t *values,
        size_t values_len) {
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
    value_t values[] = {
        {.derive = v0}, {.derive = v1},
    };

    submit(dom, type, devname, values, STATIC_ARRAY_SIZE(values));
} /* void submit_derive2 */

static int lv_init(void) {
    if (virInitialize() != 0)
        return -1;
    else
        return 0;
}

static int lv_config(const char *key, const char *value) {
    if (virInitialize() != 0)
        return 1;

    if (il_domains == NULL)
        il_domains = ignorelist_create(1);
    if (il_block_devices == NULL)
        il_block_devices = ignorelist_create(1);

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
    if (strcasecmp(key, "BlockDevice") == 0) {
        if (ignorelist_add(il_block_devices, value))
            return 1;
        return 0;
    }

    if (strcasecmp(key, "BlockDeviceFormat") == 0) {
        if (strcasecmp(value, "target") == 0)
            blockdevice_format = target;
        else if (strcasecmp(value, "source") == 0)
            blockdevice_format = source;
        else {
            ERROR(PLUGIN_NAME " plugin: unknown BlockDeviceFormat: %s", value);
            return -1;
        }
        return 0;
    }
    if (strcasecmp(key, "BlockDeviceFormatBasename") == 0) {
        blockdevice_format_basename = IS_TRUE(value);
        return 0;
    }

    if (strcasecmp(key, "IgnoreSelected") == 0) {
        if (IS_TRUE(value)) {
            ignorelist_set_invert(il_domains, 0);
            ignorelist_set_invert(il_block_devices, 0);
        } else {
            ignorelist_set_invert(il_domains, 1);
            ignorelist_set_invert(il_block_devices, 1);
        }
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
                ERROR(PLUGIN_NAME " plugin: unknown PluginInstanceFormat field: %s",
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

//tag
/**
 * This function is called when a user invokes S<C<guestfish -d guest>>.
 *
 * Returns the number of drives added (S<C<E<gt> 0>>), or C<-1> for failure.
 */
    int
add_libvirt_drives (guestfs_h *g,const char *guest)
{
    struct guestfs_add_domain_argv optargs = { .bitmask = 0 };
    /*
       if (libvirt_uri) {
       optargs.bitmask |= GUESTFS_ADD_DOMAIN_LIBVIRTURI_BITMASK;
       optargs.libvirturi = libvirt_uri;
       }*/
    //if (read_only) {
    optargs.bitmask |= GUESTFS_ADD_DOMAIN_READONLY_BITMASK;
    optargs.readonly = 1;
    //}
    /*if (live) {
      optargs.bitmask |= GUESTFS_ADD_DOMAIN_LIVE_BITMASK;
      optargs.live = 1;
      }*/

    optargs.bitmask |= GUESTFS_ADD_DOMAIN_ALLOWUUID_BITMASK;
    optargs.allowuuid = 1;

    optargs.bitmask |= GUESTFS_ADD_DOMAIN_READONLYDISK_BITMASK;
    optargs.readonlydisk = "read";

    return guestfs_add_domain_argv (g, guest, &optargs);
}

static int lv_read(void) {
    time_t t;
    char **devices = NULL;
    char **fses = NULL;
    int capacity_total;
    int capacity_free;
    char *type_instance = NULL;
    guestfs_h *g; 

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
    if ((last_refresh == (time_t)0) ||
            ((interval > 0) && ((last_refresh + interval) <= t))) {
        if (refresh_lists() != 0) {
            if (conn != NULL)
                virConnectClose(conn);
            conn = NULL;
            return -1;
        }
        last_refresh = t;
    }

    for (int i = 0; i < nr_domains; ++i) {
        g = guestfs_create ();
        if (g == NULL) {
            perror ("guestfs_create");
            return -1;
        }

        add_libvirt_drives(g,  virDomainGetName((domains[i])));
        guestfs_set_identifier (g,  virDomainGetName((domains[i]))); 
        guestfs_set_trace (g, 0);
        guestfs_set_verbose (g,0);

        if (guestfs_launch (g) == -1) {
            perror ("guestfs_launch");
            return -1;
        }

        devices = guestfs_list_devices (g);
        if (devices == NULL)
            return -1;

        fses = guestfs_list_filesystems (g);
        if (fses == NULL)
            return -1;
        //tag
#define STRNEQ(a, b) (strcmp((a),(b)) != 0)
        for (i = 0; fses[i] != NULL; i += 2) {
            if (STRNEQ (fses[i+1], "") &&
                    STRNEQ (fses[i+1], "swap") &&
                    STRNEQ (fses[i+1], "unknown")) {
                const char *dev = fses[i];
                struct guestfs_statvfs *stat = NULL;

                /* Try mounting and stating the device.  This might reasonably
                 * fail, so don't show errors.
                 */
                guestfs_push_error_handler (g, NULL, NULL);

                if (guestfs_mount_ro (g, dev, "/") == 0) {
                    stat = guestfs_statvfs (g, "/");
                    guestfs_umount_all (g);
                }
                guestfs_pop_error_handler (g);
                if (!stat){
                    continue;
                }
                capacity_total = (stat->blocks)*(stat->bsize);
                capacity_free = (stat->blocks)*(stat->frsize);
                type_instance = NULL;
                if (blockdevice_format_basename && blockdevice_format == source)
                    type_instance = strdup(basename(block_devices[i].path));
                else
                    type_instance = strdup(block_devices[i].path);
                if ((capacity_total != -1) && (capacity_free != -1))
                    submit_derive2("vm_disk_usage", (derive_t)capacity_total, (derive_t)capacity_free,
                            block_devices[i].dom, type_instance);
                sfree(type_instance);
            }
        }  } 
    return 0;
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

        free_block_devices();
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

            if (add_domain(dom) < 0) {
                ERROR(PLUGIN_NAME " plugin: malloc failed.");
                goto cont;
            }

            /* Get a list of devices for this domain. */
            xml = virDomainGetXMLDesc(dom, 0);
            if (!xml) {
                VIRT_ERROR(conn, "virDomainGetXMLDesc");
                goto cont;
            }

            /* Yuck, XML.  Parse out the devices. */
            xml_doc = xmlReadDoc((xmlChar *)xml, NULL, NULL, XML_PARSE_NONET);
            if (xml_doc == NULL) {
                VIRT_ERROR(conn, "xmlReadDoc");
                goto cont;
            }

            xpath_ctx = xmlXPathNewContext(xml_doc);

            /* Block devices. */
            char *bd_xmlpath = "/domain/devices/disk/target[@dev]";
            if (blockdevice_format == source)
                bd_xmlpath = "/domain/devices/disk/source[@dev]";
            xpath_obj = xmlXPathEval((xmlChar *)bd_xmlpath, xpath_ctx);

            if (xpath_obj == NULL || xpath_obj->type != XPATH_NODESET ||
                    xpath_obj->nodesetval == NULL)
                goto cont;

            for (int j = 0; j < xpath_obj->nodesetval->nodeNr; ++j) {
                xmlNodePtr node;
                char *path = NULL;

                node = xpath_obj->nodesetval->nodeTab[j];
                if (!node)
                    continue;
                path = (char *)xmlGetProp(node, (xmlChar *)"dev");
                if (!path)
                    continue;

                if (il_block_devices &&
                        ignore_device_match(il_block_devices, name, path) != 0)
                    goto cont2;

                add_block_device(dom, path);
cont2:
                if (path)
                    xmlFree(path);
            }
            xmlXPathFreeObject(xpath_obj);

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
    if (domains) {
        for (int i = 0; i < nr_domains; ++i)
            virDomainFree(domains[i]);
        sfree(domains);
    }
    domains = NULL;
    nr_domains = 0;
}

static int add_domain(virDomainPtr dom) {
    virDomainPtr *new_ptr;
    int new_size = sizeof(domains[0]) * (nr_domains + 1);

    if (domains)
        new_ptr = realloc(domains, new_size);
    else
        new_ptr = malloc(new_size);

    if (new_ptr == NULL)
        return -1;

    domains = new_ptr;
    domains[nr_domains] = dom;
    return nr_domains++;
}

static void free_block_devices(void) {
    if (block_devices) {
        for (int i = 0; i < nr_block_devices; ++i)
            sfree(block_devices[i].path);
        sfree(block_devices);
    }
    block_devices = NULL;
    nr_block_devices = 0;
}

static int add_block_device(virDomainPtr dom, const char *path) {
    struct block_device *new_ptr;
    int new_size = sizeof(block_devices[0]) * (nr_block_devices + 1);
    char *path_copy;

    path_copy = strdup(path);
    if (!path_copy)
        return -1;

    if (block_devices)
        new_ptr = realloc(block_devices, new_size);
    else
        new_ptr = malloc(new_size);

    if (new_ptr == NULL) {
        sfree(path_copy);
        return -1;
    }
    block_devices = new_ptr;
    block_devices[nr_block_devices].dom = dom;
    block_devices[nr_block_devices].path = path_copy;
    return nr_block_devices++;
}

static int ignore_device_match(ignorelist_t *il, const char *domname,
        const char *devpath) {
    char *name;
    int n, r;

    if ((domname == NULL) || (devpath == NULL))
        return 0;

    n = sizeof(char) * (strlen(domname) + strlen(devpath) + 2);
    name = malloc(n);
    if (name == NULL) {
        ERROR(PLUGIN_NAME " plugin: malloc failed.");
        return 0;
    }
    ssnprintf(name, n, "%s:%s", domname, devpath);
    r = ignorelist_match(il, name);
    sfree(name);
    return r;
}

static int lv_shutdown(void) {
    free_block_devices();
    free_domains();

    if (conn != NULL)
        virConnectClose(conn);
    conn = NULL;

    ignorelist_free(il_domains);
    il_domains = NULL;
    ignorelist_free(il_block_devices);
    il_block_devices = NULL;
    return 0;
}

void module_register(void) {
    plugin_register_config(PLUGIN_NAME, lv_config, config_keys, NR_CONFIG_KEYS);
    plugin_register_init(PLUGIN_NAME, lv_init);
    plugin_register_read(PLUGIN_NAME, lv_read);
    plugin_register_shutdown(PLUGIN_NAME, lv_shutdown);
}

/*
 * vim: shiftwidth=4 tabstop=8 softtabstop=4 expandtab fdm=marker
 */
