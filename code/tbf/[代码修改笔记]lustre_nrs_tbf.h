/*
 *
 * Network Request Scheduler (NRS) Token Bucket Filter(TBF) policy
 *
 */

#ifndef _LUSTRE_NRS_TBF_H
#define _LUSTRE_NRS_TBF_H

/* \name tbf
 *
 * TBF policy
 *
 * @{
 */

struct nrs_tbf_head;
struct nrs_tbf_cmd;

struct nrs_tbf_jobid {
	char		*tj_id;
	struct list_head tj_linkage;
};

struct nrs_tbf_client {
	/** Resource object for policy instance. */
	struct ptlrpc_nrs_resource	 tc_res;
	/** Node in the hash table. */
	struct hlist_node		 tc_hnode;
	/** NID of the client.<===>ptlrpc_request req->rq_peer.nid */
	lnet_nid_t			 tc_nid;
	/** Jobid of the client. */
	char				 tc_jobid[LUSTRE_JOBID_SIZE];
	/** Reference number of the client. */
	atomic_t			 tc_ref;
	/** Lock to protect rule and linkage. */
	spinlock_t			 tc_rule_lock;
	/** Linkage to rule. 可以挂在nrs_tbf_rule的tr_cli_list*/
	struct list_head	         tc_linkage;
	/** Pointer to rule. */ 
    //和令牌的产生规则和使用相关的  
	struct nrs_tbf_rule		*tc_rule;
    
    //下面的这些参数都是根据tc_rule中赋值的，详见nrs_tbf_cli_reset_value函数
    
	/** Generation of the rule matched. */
	__u64				 tc_rule_generation;
	/** Limit of RPC rate. */
	//rule->tr_nsecs = NSEC_PER_SEC / rule->tr_rpc_rate;
	__u64				 tc_rpc_rate;
	/** Time to wait for next token. */
	__u64				 tc_nsecs;
	/** RPC token number. */
	__u64				 tc_ntoken;
	/** Token bucket depth. */
    //暂时理解为对获得tokens的一个限制,获得的tokens超过tc_depth都将为tc_depth
	__u64				 tc_depth;
	/** Time check-point. */
	__u64				 tc_check_time;
	/** List of queued requests.===> nrs_tbf_req */
	struct list_head		 tc_list;
	/** Node in binary heap.client通过一个二叉堆维持着 */
	cfs_binheap_node_t		 tc_node;
	/** Whether the client is in heap. */
	bool				 tc_in_heap;
	/** Sequence of the newest rule. */
	__u32				 tc_rule_sequence;
	/**
	 * Linkage into LRU list. Protected bucket lock of
	 * nrs_tbf_head::th_cli_hash.
	 */
	struct list_head		 tc_lru;
};

#define MAX_TBF_NAME (16)

#define NTRS_STOPPING	0x0000001
#define NTRS_DEFAULT	0x0000002

struct nrs_tbf_rule {
	/** Name of the rule. */
	char				 tr_name[MAX_TBF_NAME];
	/** Head belongs to. */
	struct nrs_tbf_head		*tr_head;
	/** Likage to head. */
	struct list_head		 tr_linkage;
	/** Nid list of the rule. */
	struct list_head		 tr_nids;
	/** Nid list string of the rule.*/
	char				*tr_nids_str;
	/** Jobid list of the rule. */
	struct list_head		 tr_jobids;
	/** Jobid list string of the rule.*/
	char				*tr_jobids_str;
	/** RPC/s limit. */
	__u64				 tr_rpc_rate;
	/** Time to wait for next token. */
	__u64				 tr_nsecs;
	/** Token bucket depth. */
	__u64				 tr_depth;
	/** Lock to protect the list of clients. */
	spinlock_t			 tr_rule_lock;
	/** List of client. */
	struct list_head		 tr_cli_list;
	/** Flags of the rule. */
	__u32				 tr_flags;
	/** Usage Reference count taken on the rule. */
	atomic_t			 tr_ref;
	/** Generation of the rule. */
	__u64				 tr_generation;
};

struct nrs_tbf_ops {
	char *o_name;
	int (*o_startup)(struct ptlrpc_nrs_policy *, struct nrs_tbf_head *);
	struct nrs_tbf_client *(*o_cli_find)(struct nrs_tbf_head *,
					     struct ptlrpc_request *);
	struct nrs_tbf_client *(*o_cli_findadd)(struct nrs_tbf_head *,
						struct nrs_tbf_client *);
	void (*o_cli_put)(struct nrs_tbf_head *, struct nrs_tbf_client *);
	void (*o_cli_init)(struct nrs_tbf_client *, struct ptlrpc_request *);
	int (*o_rule_init)(struct ptlrpc_nrs_policy *,
			   struct nrs_tbf_rule *,
			   struct nrs_tbf_cmd *);
	int (*o_rule_dump)(struct nrs_tbf_rule *, struct seq_file *);
	int (*o_rule_match)(struct nrs_tbf_rule *,
			    struct nrs_tbf_client *);
	void (*o_rule_fini)(struct nrs_tbf_rule *);
};

#define NRS_TBF_TYPE_JOBID	"jobid"
#define NRS_TBF_TYPE_NID	"nid"
#define NRS_TBF_TYPE_MAX_LEN	20
#define NRS_TBF_FLAG_JOBID	0x0000001
#define NRS_TBF_FLAG_NID	0x0000002

//在nrs_tbf_jobid_cli_put中会调用到
//其实就是一个普通的hash bucket,利用其性质刚好可以做lru
struct nrs_tbf_bucket {
	/**
	 * LRU list, updated on each access to client. Protected by
	 * bucket lock of nrs_tbf_head::th_cli_hash.
	 */
	struct list_head	ntb_lru;
};

/**
 * Private data structure for the TBF policy
 */
struct nrs_tbf_head {
	/**
	 * Resource object for policy instance.
	 */
	struct ptlrpc_nrs_resource	 th_res;
	/**
	 * List of rules.
	 */
	struct list_head		 th_list;
	/**
	 * Lock to protect the list of rules.
	 */
	spinlock_t			 th_rule_lock;
	/**
	 * Generation of rules.
	 */
	atomic_t			 th_rule_sequence;
	/**
	 * Default rule.
	 */
	struct nrs_tbf_rule		*th_rule;
	/**
	 * Timer for next token.
	 */
	struct hrtimer			 th_timer;
	/**
	 * Deadline of the timer.
	 */
	__u64				 th_deadline;
	/**
	 * Sequence of requests.
	 */
	__u64				 th_sequence;
	/**
	 * Heap of queues.
	 */
	cfs_binheap_t			*th_binheap;
	/**
	 * Hash of clients.
	 */
	struct cfs_hash			*th_cli_hash;
	/**
	 * Type of TBF policy.
	 */
	char				 th_type[NRS_TBF_TYPE_MAX_LEN + 1];
	/**
	 * Rule operations.
	 */
	struct nrs_tbf_ops		*th_ops;
	/**
	 * Flag of type.
	 */
	__u32				 th_type_flag;
	/**
	 * Index of bucket on hash table while purging.
	 */
	int				 th_purge_start;
};

enum nrs_tbf_cmd_type {
	NRS_CTL_TBF_START_RULE = 0,
	NRS_CTL_TBF_STOP_RULE,
	NRS_CTL_TBF_CHANGE_RATE,
};

struct nrs_tbf_cmd {
	enum nrs_tbf_cmd_type	 tc_cmd;
	char			*tc_name;
	__u64			 tc_rpc_rate;
	struct list_head	 tc_nids;
	char			*tc_nids_str;
	struct list_head	 tc_jobids;
	char			*tc_jobids_str;
	__u32			 tc_valid_types;
	__u32			 tc_rule_flags;
};

struct nrs_tbf_req {
	/**
	 * Linkage to queue.
	 */
	struct list_head	tr_list;
	/**
	 * Sequence of the request.
	 */
	__u64			tr_sequence;
};

/**
 * TBF policy operations.
 */
enum nrs_ctl_tbf {
	/**
	 * Read the the data of a TBF policy.
	 */
	NRS_CTL_TBF_RD_RULE = PTLRPC_NRS_CTL_1ST_POL_SPEC,
	/**
	 * Write the the data of a TBF policy.
	 */
	NRS_CTL_TBF_WR_RULE,
};

/** @} tbf */
#endif
