/*
 * GPL HEADER START
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 only,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License version 2 for more details (a copy is included
 * in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; If not, see
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 * GPL HEADER END
 */
/*
 * Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
 * Use is subject to license terms.
 *
 * Copyright (c) 2012, 2014, Intel Corporation.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 *
 * libcfs/include/libcfs/libcfs_debug.h
 *
 * Debug messages and assertions
 *
 */

#ifndef __LIBCFS_DEBUG_H__
#define __LIBCFS_DEBUG_H__

/*
 *  Debugging
 */
extern unsigned int libcfs_subsystem_debug;
extern unsigned int libcfs_stack;
extern unsigned int libcfs_debug;
extern unsigned int libcfs_printk;
extern unsigned int libcfs_console_ratelimit;
extern unsigned int libcfs_watchdog_ratelimit;
extern unsigned int libcfs_console_max_delay;
extern unsigned int libcfs_console_min_delay;
extern unsigned int libcfs_console_backoff;
extern unsigned int libcfs_debug_binary;
extern char libcfs_debug_file_path_arr[PATH_MAX];

int libcfs_debug_mask2str(char *str, int size, int mask, int is_subsys);
int libcfs_debug_str2mask(int *mask, const char *str, int is_subsys);

/* Has there been an LBUG? */
extern unsigned int libcfs_catastrophe;
extern unsigned int libcfs_panic_on_lbug;

/**
 * Format for debug message headers
 */
struct ptldebug_header {
        __u32 ph_len;
        __u32 ph_flags;
        __u32 ph_subsys;
        __u32 ph_mask;
        __u16 ph_cpu_id;
        __u16 ph_type;
        __u32 ph_sec;
        __u64 ph_usec;
        __u32 ph_stack;
        __u32 ph_pid;
        __u32 ph_extern_pid;
        __u32 ph_line_num;
} __attribute__((packed));


#define PH_FLAG_FIRST_RECORD 1

/* Debugging subsystems (32 bits, non-overlapping) */
#define S_UNDEFINED	0x00000001
#define S_MDC		0x00000002
#define S_MDS		0x00000004
#define S_OSC		0x00000008
#define S_OST		0x00000010
#define S_CLASS		0x00000020
#define S_LOG		0x00000040
#define S_LLITE		0x00000080
#define S_RPC		0x00000100
#define S_MGMT		0x00000200
#define S_LNET		0x00000400
#define S_LND		0x00000800 /* ALL LNDs */
#define S_PINGER	0x00001000
#define S_FILTER	0x00002000
/* unused */
#define S_ECHO		0x00008000
#define S_LDLM		0x00010000
#define S_LOV		0x00020000
#define S_LQUOTA	0x00040000
#define S_OSD		0x00080000
#define S_LFSCK		0x00100000
/* unused */
/* unused */
#define S_LMV		0x00800000 /* b_new_cmd */
/* unused */
#define S_SEC		0x02000000 /* upcall cache */
#define S_GSS		0x04000000 /* b_new_cmd */
/* unused */
#define S_MGC		0x10000000
#define S_MGS		0x20000000
#define S_FID		0x40000000 /* b_new_cmd */
#define S_FLD		0x80000000 /* b_new_cmd */

#define LIBCFS_DEBUG_SUBSYS_NAMES {					\
	"undefined", "mdc", "mds", "osc", "ost", "class", "log",	\
	"llite", "rpc", "mgmt", "lnet", "lnd", "pinger", "filter", "",	\
	"echo", "ldlm", "lov", "lquota", "osd", "lfsck", "", "", "lmv",	\
	 "", "sec", "gss", "", "mgc", "mgs", "fid", "fld", NULL }

/* Debugging masks (32 bits, non-overlapping) */
#define D_TRACE		0x00000001 /* ENTRY/EXIT markers */
#define D_INODE		0x00000002
#define D_SUPER		0x00000004
#define D_EXT2		0x00000008 /* anything from ext2_debug */
#define D_MALLOC	0x00000010 /* print malloc, free information */
#define D_CACHE		0x00000020 /* cache-related items */
#define D_INFO		0x00000040 /* general information */
#define D_IOCTL		0x00000080 /* ioctl related information */
#define D_NETERROR	0x00000100 /* network errors */
#define D_NET		0x00000200 /* network communications */
#define D_WARNING	0x00000400 /* CWARN(...) == CDEBUG (D_WARNING, ...) */
#define D_BUFFS		0x00000800
#define D_OTHER		0x00001000
#define D_DENTRY	0x00002000
#define D_NETTRACE	0x00004000
#define D_PAGE		0x00008000 /* bulk page handling */
#define D_DLMTRACE	0x00010000
#define D_ERROR		0x00020000 /* CERROR(...) == CDEBUG (D_ERROR, ...) */
#define D_EMERG		0x00040000 /* CEMERG(...) == CDEBUG (D_EMERG, ...) */
#define D_HA		0x00080000 /* recovery and failover */
#define D_RPCTRACE	0x00100000 /* for distributed debugging */
#define D_VFSTRACE	0x00200000
#define D_READA		0x00400000 /* read-ahead */
#define D_MMAP		0x00800000
#define D_CONFIG	0x01000000
#define D_CONSOLE	0x02000000
#define D_QUOTA		0x04000000
#define D_SEC		0x08000000
#define D_LFSCK		0x10000000 /* For both OI scrub and LFSCK */
#define D_HSM		0x20000000

#define LIBCFS_DEBUG_MASKS_NAMES {					\
	"trace", "inode", "super", "ext2", "malloc", "cache", "info",	\
	"ioctl", "neterror", "net", "warning", "buffs", "other",	\
	"dentry", "nettrace", "page", "dlmtrace", "error", "emerg",	\
	"ha", "rpctrace", "vfstrace", "reada", "mmap", "config",	\
	"console", "quota", "sec", "lfsck", "hsm", NULL }

#define D_CANTMASK   (D_ERROR | D_EMERG | D_WARNING | D_CONSOLE)

#ifndef DEBUG_SUBSYSTEM
# define DEBUG_SUBSYSTEM S_UNDEFINED
#endif

#define CDEBUG_DEFAULT_MAX_DELAY (cfs_time_seconds(600))         /* jiffies */
#define CDEBUG_DEFAULT_MIN_DELAY ((cfs_time_seconds(1) + 1) / 2) /* jiffies */
#define CDEBUG_DEFAULT_BACKOFF   2
struct cfs_debug_limit_state {
	unsigned long	cdls_next;
	unsigned int	cdls_delay;
	int		cdls_count;
};

struct libcfs_debug_msg_data {
	const char			*msg_file;
	const char			*msg_fn;
	int				 msg_subsys;
	int				 msg_line;
	int				 msg_mask;
	struct cfs_debug_limit_state	*msg_cdls;
};

#define LIBCFS_DEBUG_MSG_DATA_INIT(data, mask, cdls)        \
do {                                                        \
        (data)->msg_subsys = DEBUG_SUBSYSTEM;               \
        (data)->msg_file   = __FILE__;                      \
        (data)->msg_fn     = __FUNCTION__;                  \
        (data)->msg_line   = __LINE__;                      \
        (data)->msg_cdls   = (cdls);                        \
        (data)->msg_mask   = (mask);                        \
} while (0)

#define LIBCFS_DEBUG_MSG_DATA_DECL(dataname, mask, cdls)    \
        static struct libcfs_debug_msg_data dataname = {    \
               .msg_subsys = DEBUG_SUBSYSTEM,               \
               .msg_file   = __FILE__,                      \
               .msg_fn     = __FUNCTION__,                  \
               .msg_line   = __LINE__,                      \
               .msg_cdls   = (cdls)         };              \
        dataname.msg_mask   = (mask);

#ifdef __KERNEL__

# ifdef CDEBUG_ENABLED

/**
 * Filters out logging messages based on mask and subsystem.
 */
static inline int cfs_cdebug_show(unsigned int mask, unsigned int subsystem)
{
        return mask & D_CANTMASK ||
                ((libcfs_debug & mask) && (libcfs_subsystem_debug & subsystem));
}

#  define __CDEBUG(cdls, mask, format, ...)				\
do {                                                                    \
        static struct libcfs_debug_msg_data msgdata;                    \
                                                                        \
        CFS_CHECK_STACK(&msgdata, mask, cdls);                          \
                                                                        \
        if (cfs_cdebug_show(mask, DEBUG_SUBSYSTEM)) {                   \
                LIBCFS_DEBUG_MSG_DATA_INIT(&msgdata, mask, cdls);       \
                libcfs_debug_msg(&msgdata, format, ## __VA_ARGS__);     \
        }                                                               \
} while (0)

#  define CDEBUG(mask, format, ...) __CDEBUG(NULL, mask, format, ## __VA_ARGS__)

#  define CDEBUG_LIMIT(mask, format, ...)				\
do {									\
	static struct cfs_debug_limit_state cdls;			\
									\
	__CDEBUG(&cdls, mask, format, ## __VA_ARGS__);			\
} while (0)

# else /* !CDEBUG_ENABLED */
static inline int cfs_cdebug_show(unsigned int mask, unsigned int subsystem)
{
        return 0;
}
#  define CDEBUG(mask, format, ...) (void)(0)
#  define CDEBUG_LIMIT(mask, format, ...) (void)(0)
#  warning "CDEBUG IS DISABLED. THIS SHOULD NEVER BE DONE FOR PRODUCTION!"
# endif /* CDEBUG_ENABLED */

#else /* !__KERNEL__ */
static inline int cfs_cdebug_show(unsigned int mask, unsigned int subsystem)
{
        return 0;
}
# define CDEBUG(mask, format, ...)					\
do {                                                                    \
        if (((mask) & D_CANTMASK) != 0)                                 \
                fprintf(stderr, "(%s:%d:%s()) " format,                 \
                        __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__);\
} while (0)

# define CDEBUG_LIMIT CDEBUG

#endif /* __KERNEL__ */

/*
 * Lustre Error Checksum: calculates checksum
 * of Hex number by XORing each bit.
 */
#define LERRCHKSUM(hexnum) (((hexnum) & 0xf) ^ ((hexnum) >> 4 & 0xf) ^ \
			   ((hexnum) >> 8 & 0xf))

#define CWARN(format, ...)          CDEBUG_LIMIT(D_WARNING, format, ## __VA_ARGS__)
#define CERROR(format, ...)         CDEBUG_LIMIT(D_ERROR, format, ## __VA_ARGS__)
#define CNETERR(format, a...)       CDEBUG_LIMIT(D_NETERROR, format, ## a)
#define CEMERG(format, ...)         CDEBUG_LIMIT(D_EMERG, format, ## __VA_ARGS__)

#define LCONSOLE(mask, format, ...) CDEBUG(D_CONSOLE | (mask), format, ## __VA_ARGS__)
#define LCONSOLE_INFO(format, ...)  CDEBUG_LIMIT(D_CONSOLE, format, ## __VA_ARGS__)
#define LCONSOLE_WARN(format, ...)  CDEBUG_LIMIT(D_CONSOLE | D_WARNING, format, ## __VA_ARGS__)
#define LCONSOLE_ERROR_MSG(errnum, format, ...) CDEBUG_LIMIT(D_CONSOLE | D_ERROR, \
                           "%x-%x: " format, errnum, LERRCHKSUM(errnum), ## __VA_ARGS__)
#define LCONSOLE_ERROR(format, ...) LCONSOLE_ERROR_MSG(0x00, format, ## __VA_ARGS__)

#define LCONSOLE_EMERG(format, ...) CDEBUG(D_CONSOLE | D_EMERG, format, ## __VA_ARGS__)

#if defined(CDEBUG_ENTRY_EXIT) && defined(__KERNEL__)

void libcfs_log_goto(struct libcfs_debug_msg_data *goto_data,
		     const char *label, long rc);

# define GOTO(label, rc)						\
do {									\
	if (cfs_cdebug_show(D_TRACE, DEBUG_SUBSYSTEM)) {		\
		LIBCFS_DEBUG_MSG_DATA_DECL(_goto_data, D_TRACE, NULL);	\
		libcfs_log_goto(&_goto_data, #label, (long)(rc));	\
	} else {							\
		(void)(rc);						\
	}								\
									\
	goto label;							\
} while (0)


long libcfs_log_return(struct libcfs_debug_msg_data *, long rc);
# if BITS_PER_LONG > 32
#  define RETURN(rc)							\
do {									\
	if (cfs_cdebug_show(D_TRACE, DEBUG_SUBSYSTEM)) {		\
                LIBCFS_DEBUG_MSG_DATA_DECL(msgdata, D_TRACE, NULL);	\
                return (typeof(rc))libcfs_log_return(&msgdata,		\
                                                     (long)(rc));	\
	}								\
									\
	return (rc);							\
} while (0)
# else /* BITS_PER_LONG == 32 */
/* We need an on-stack variable, because we cannot case a 32-bit pointer
 * directly to (long long) without generating a complier warning/error, yet
 * casting directly to (long) will truncate 64-bit return values. The log
 * values will print as 32-bit values, but they always have been. LU-1436
 */
#  define RETURN(rc)							\
do {									\
	if (cfs_cdebug_show(D_TRACE, DEBUG_SUBSYSTEM)) {		\
		typeof(rc) __rc = (rc);					\
		LIBCFS_DEBUG_MSG_DATA_DECL(msgdata, D_TRACE, NULL);	\
		libcfs_log_return(&msgdata, (long)__rc);		\
		return __rc;						\
	}								\
									\
	return (rc);							\
} while (0)

# endif /* BITS_PER_LONG > 32 */

# define ENTRY	CDEBUG(D_TRACE, "Process entered\n")
# define EXIT	CDEBUG(D_TRACE, "Process leaving\n")

#else /* !CDEBUG_ENTRY_EXIT || !__KERNEL__ */

# define GOTO(label, rc)						\
	do {								\
		((void)(rc));						\
		goto label;						\
	} while (0)

# define RETURN(rc) return (rc)
# define ENTRY	do { } while (0)
# define EXIT	do { } while (0)

#endif /* CDEBUG_ENTRY_EXIT && __KERNEL__ */

#define RETURN_EXIT							\
do {									\
	EXIT;								\
	return;								\
} while (0)

extern int libcfs_debug_msg(struct libcfs_debug_msg_data *msgdata,
                            const char *format1, ...)
        __attribute__ ((format (printf, 2, 3)));

extern int libcfs_debug_vmsg2(struct libcfs_debug_msg_data *msgdata,
                              const char *format1,
                              va_list args, const char *format2, ...)
        __attribute__ ((format (printf, 4, 5)));

#ifdef __KERNEL__
/* other external symbols that tracefile provides: */
extern int cfs_trace_copyin_string(char *knl_buffer, int knl_buffer_nob,
				   const char __user *usr_buffer,
				   int usr_buffer_nob);
extern int cfs_trace_copyout_string(char __user *usr_buffer, int usr_buffer_nob,
				    const char *knl_buffer, char *append);
#endif /* __KERNEL__ */

#define LIBCFS_DEBUG_FILE_PATH_DEFAULT "/tmp/lustre-log"

#endif	/* __LIBCFS_DEBUG_H__ */