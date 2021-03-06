// Module.markers
// cc.1.8.patch
// cc.patch
// build/Module.markers
// ldiskfs/Module.markers
// lustre/lov/.lov:q.swp
// lustre/ost/ost_nrs.c
// lustre/ost/ost_nrs.c-bak
//ndex: lustre/include/lustre_export.h
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/include/lustre_export.h,v
//etrieving revision 1.4.6.9.2.1.2.12
//iff -p -u -r1.4.6.9.2.1.2.12 lustre_export.h
//-- lustre/include/lustre_export.h	18 Nov 2008 21:39:58 -0000	1.4.6.9.2.1.2.12
//++ lustre/include/lustre_export.h	1 Mar 2010 11:39:58 -0000
//@ -123,6 +123,7 @@ struct obd_export {
         struct portals_handle     exp_handle;
         atomic_t                  exp_refcount;
         atomic_t                  exp_rpc_count;
        atomic_t                  exp_rw_in_flight;
         struct obd_uuid           exp_client_uuid;
         lnet_nid_t                exp_client_nid;
         struct list_head          exp_obd_chain;
//@ -142,6 +143,7 @@ struct obd_export {
         struct list_head          exp_uncommitted_replies;
         spinlock_t                exp_uncommitted_replies_lock;
         time_t                    exp_last_request_time;
        time_t                    exp_last_active_time;
         struct list_head          exp_req_replay_queue;
         spinlock_t                exp_lock; /* protects flags int below */
         /* ^ protects exp_outstanding_replies too */
//@ -157,7 +159,8 @@ struct obd_export {
                                   exp_vbr_failed:1,
                                   exp_replay_needed:1,
                                   exp_need_sync:1, /* needs sync from connect */
//                                  exp_libclient:1; /* liblustre client? */
                                  exp_libclient:1,
                                  exp_rw_active; /* liblustre client? */
         struct list_head          exp_queued_rpc;  /* RPC to be handled */
         /* VBR: per-export last committed */
         __u64                     exp_last_committed;
//ndex: lustre/include/lustre_net.h
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/include/lustre_net.h,v
//etrieving revision 1.9.6.28.4.16.2.1
//iff -p -u -r1.9.6.28.4.16.2.1 lustre_net.h
//-- lustre/include/lustre_net.h	25 Mar 2009 03:58:46 -0000	1.9.6.28.4.16.2.1
//++ lustre/include/lustre_net.h	1 Mar 2010 11:40:29 -0000
//@ -292,6 +292,22 @@ struct ptlrpc_hpreq_ops {
         int  (*hpreq_check)(struct ptlrpc_request *);
//};
//struct ptlrpc_nrs_node {
#if defined(__KERNEL__)
        struct rb_node     pnn_node;
#else
        struct rb_node {
                unsigned long rb_parent_color;
                struct rb_node *rb_right;
                struct rb_node *rb_left;
        } pnn_node;
#endif
        __u64             pnn_start;
        __u64             pnn_end;
        void             *pnn_data;
        struct obd_ioobj *pnn_obj;
};

//struct ptlrpc_request {
         int rq_type; /* one of PTL_RPC_MSG_* */
         struct list_head rq_list;
//@ -299,7 +315,7 @@ struct ptlrpc_request {
         struct list_head rq_history_list;       /* server-side history */
         struct list_head rq_exp_list;           /* server-side per-export list */
         struct ptlrpc_hpreq_ops *rq_ops;        /* server-side hp handlers */
//
        struct ptlrpc_nrs_node rq_nrs_node;     /* server-side nrs node */
         __u64            rq_history_seq;        /* history sequence # */
         int rq_status;
         spinlock_t rq_lock;
//@ -321,7 +337,8 @@ struct ptlrpc_request {
                 /* server-side flags */
                 rq_packed_final:1,  /* packed final reply */
                 rq_sent_final:1,    /* stop sending early replies */
//                rq_hp:1;            /* high priority RPC */
                rq_hp:1,            /* high priority RPC */
                rq_nrs:1;           /* link to nrs list */
         enum rq_phase rq_phase;     /* one of RQ_PHASE_* */
         enum rq_phase rq_next_phase; /* one of RQ_PHASE_* to be used next */
         atomic_t rq_refcount;   /* client-side refcount for SENT race,
//@ -358,6 +375,8 @@ struct ptlrpc_request {
//         /* server-side... */
         struct timeval       rq_arrival_time;       /* request arrival time */
        long                 rq_wait_time;
        long                 rq_work_time;
         struct ptlrpc_reply_state *rq_reply_state;  /* separated reply state */
         struct ptlrpc_request_buffer_desc *rq_rqbd; /* incoming request buffer*/
//#ifdef CRAY_XT3
//@ -575,6 +594,72 @@ typedef int (*svc_handler_t)(struct ptlr
//typedef void (*svcreq_printfn_t)(void *, struct ptlrpc_request *);
//typedef int (*svc_hpreq_handler_t)(struct ptlrpc_request *);
//#define REQ_NRS_SKIP    0
#define REQ_NRS_ACCEPT  1

struct nrs;
struct nrs_ops {
        /**
         * add an RPC request to the NRS:
         * return 1, the request is handled by NRS;
         * return 0, the request is skipped by NRS.
         */
        int (*nrs_enqueue)(struct nrs *s, struct ptlrpc_request *);

        /**
         * dequeue a request from NRS
         */
        struct ptlrpc_request *(*nrs_dequeue)(struct nrs *s);

        /**
         * priority a request
         */
        void (*nrs_priority)(struct nrs *s, struct ptlrpc_request *,
                             struct list_head *priority_list);
        int (*nrs_get_rcc)(struct nrs *s, int nr);
        /**
         * hook fn upon the completion of a req
         */
        void (*nrs_finish)(struct nrs *s, struct ptlrpc_request *);

        void (*nrs_stat_cleanup)(struct nrs *s);
        int (*nrs_empty)(struct nrs *s);

        int (*nrs_update_clnr)(struct nrs *s, struct ptlrpc_request *);
        int (*nrs_init)(struct nrs *);
        void (*nrs_exit)(struct nrs *);
};

/*
 * NRS algorithm
 */
#define NRS_NAME_MAX (16)
struct nrs_type {
        struct list_head nt_list;
        struct nrs_ops   nt_ops;
        char             nt_name[NRS_NAME_MAX];
};

/**
 * server-side Network Request Scheduler (NRS)
 */
struct nrs {
        int                    nrs_on;
        struct nrs_ops        *nrs_ops;
        struct nrs_type       *nrs_type;
        spinlock_t            *nrs_lock;
        obd_count              nrs_queued_num;
        obd_count              nrs_max_depth;
        long                   nrs_max_serv_time;
        void                  *nrs_data;

        void                   (*nrs_destory)(struct ptlrpc_service *);

        struct ptlrpc_service *nrs_svc;
        struct lprocfs_stats  *nrs_stats;
        cfs_proc_dir_entry_t  *nrs_proc_entry;
};

//#define PTLRPC_SVC_HP_RATIO 10
////struct ptlrpc_service {
//@ -638,6 +723,9 @@ struct ptlrpc_service {
         svc_handler_t      srv_handler;
         svc_hpreq_handler_t srv_hpreq_handler;  /* hp request handler */
//        struct nrs      *srv_nrs;               /* server-side request scheduler */
        struct nrs_type *srv_nrs_type;

         char *srv_name;  /* only statically allocated strings here; we don't clean them */
         char *srv_thread_name;  /* only statically allocated strings here; we don't clean them */
////@ -663,6 +751,7 @@ struct ptlrpc_service {
         void (*srv_done)(struct ptlrpc_thread *thread);
//         //struct ptlrpc_srv_ni srv_interfaces[0];
        struct ptlrpc_service *srv_rw;
//};
////struct ptlrpcd_ctl {
//ndex: lustre/include/lustre/lustre_idl.h
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/include/lustre/lustre_idl.h,v
//etrieving revision 1.3.12.26.2.3.2.17.2.6
//iff -p -u -r1.3.12.26.2.3.2.17.2.6 lustre_idl.h
//-- lustre/include/lustre/lustre_idl.h	7 Apr 2009 07:26:20 -0000	1.3.12.26.2.3.2.17.2.6
//++ lustre/include/lustre/lustre_idl.h	1 Mar 2010 11:40:50 -0000
//@ -2071,6 +2071,7 @@ struct obdo {
//#define o_undirty o_mode
//#define o_dropped o_misc
//#define o_cksum   o_nlink
#define o_rcc     o_padding_1;
////extern void lustre_swab_obdo (struct obdo *o);
////ndex: lustre/ldlm/ldlm_lib.c
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/ldlm/ldlm_lib.c,v
//etrieving revision 1.96.6.38.2.2.2.25.2.14
//iff -p -u -r1.96.6.38.2.2.2.25.2.14 ldlm_lib.c
//-- lustre/ldlm/ldlm_lib.c	14 Apr 2009 18:18:13 -0000	1.96.6.38.2.2.2.25.2.14
//++ lustre/ldlm/ldlm_lib.c	1 Mar 2010 11:41:08 -0000
//@ -1922,12 +1922,19 @@ target_send_reply(struct ptlrpc_request
////int target_handle_ping(struct ptlrpc_request *req)
//{
        struct ptlrpc_service *svc;

         if (lustre_msg_get_flags(req->rq_reqmsg) & MSG_LAST_REPLAY &&
             req->rq_export->exp_in_recovery) {
                 spin_lock(&req->rq_export->exp_lock);
                 req->rq_export->exp_in_recovery = 0;
                 spin_unlock(&req->rq_export->exp_lock);
         }

        svc = req->rq_rqbd->rqbd_service;
        if (svc->srv_rw && svc->srv_rw->srv_nrs != NULL)
                svc->srv_rw->srv_nrs->nrs_ops->nrs_update_clnr(svc->srv_rw->srv_nrs, req);

         obd_ping(req->rq_export);
         return lustre_pack_reply(req, 1, NULL, NULL);
//}
//ndex: lustre/obdclass/genops.c
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/obdclass/genops.c,v
//etrieving revision 1.145.6.22.2.3.2.15.4.3
//iff -p -u -r1.145.6.22.2.3.2.15.4.3 genops.c
//-- lustre/obdclass/genops.c	16 Apr 2009 07:16:42 -0000	1.145.6.22.2.3.2.15.4.3
//++ lustre/obdclass/genops.c	1 Mar 2010 11:41:17 -0000
//@ -705,6 +705,7 @@ struct obd_export *class_new_export(stru
         export->exp_lock_hash = NULL;
         atomic_set(&export->exp_refcount, 2);
         atomic_set(&export->exp_rpc_count, 0);
        atomic_set(&export->exp_rw_in_flight, 0);
         export->exp_obd = obd;
         CFS_INIT_LIST_HEAD(&export->exp_outstanding_replies);
         spin_lock_init(&export->exp_uncommitted_replies_lock);
//@ -715,6 +716,7 @@ struct obd_export *class_new_export(stru
         CFS_INIT_LIST_HEAD(&export->exp_handle.h_link);
         class_handle_hash(&export->exp_handle, export_handle_addref);
         export->exp_last_request_time = cfs_time_current_sec();
        export->exp_last_active_time = cfs_time_current_sec();
         spin_lock_init(&export->exp_lock);
         INIT_HLIST_NODE(&export->exp_uuid_hash);
         INIT_HLIST_NODE(&export->exp_nid_hash);
//ndex: lustre/osc/osc_request.c
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/osc/osc_request.c,v
//etrieving revision 1.284.6.49.2.5.2.23.2.11
//iff -p -u -r1.284.6.49.2.5.2.23.2.11 osc_request.c
//-- lustre/osc/osc_request.c	20 Apr 2009 05:14:49 -0000	1.284.6.49.2.5.2.23.2.11
//++ lustre/osc/osc_request.c	1 Mar 2010 11:41:44 -0000
//@ -847,6 +847,11 @@ static void osc_update_grant(struct clie
         CDEBUG(D_CACHE, "got "LPU64" extra grant\n", body->oa.o_grant);
         if (body->oa.o_valid & OBD_MD_FLGRANT)
                 cli->cl_avail_grant += body->oa.o_grant;

        /* Congest control: update feedback request concurrency credits. */
        if (body->oa.o_padding_1 > 0)
                cli->cl_max_rpcs_in_flight = body->oa.o_padding_1;

         /* waiters are woken in brw_interpret */
         client_obd_list_unlock(&cli->cl_loi_list_lock);
//}
//@ -1126,6 +1131,7 @@ static int osc_brw_prep_request(int cmd,
                 ptlrpc_req_set_repsize(req, 2, size);
         }
//        body->oa.o_padding_1 = cli->cl_dirty >> 20;
         CLASSERT(sizeof(*aa) <= sizeof(req->rq_async_args));
         aa = ptlrpc_req_async_args(req);
         aa->aa_oa = oa;
//@ -2348,6 +2354,9 @@ static int osc_send_oap_rpc(struct clien
         else
                 cli->cl_w_in_flight++;
//        if (cli->cl_pending_w_pages + cli->cl_pending_r_pages == 0)
                cli->cl_max_rpcs_in_flight = 1;

         /* queued sync pages can be torn down while the pages
          * were between the pending list and the rpc */
         tmp = NULL;
//ndex: lustre/ost/Makefile.in
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/ost/Makefile.in,v
//etrieving revision 1.4
//iff -p -u -r1.4 Makefile.in
//-- lustre/ost/Makefile.in	2 Jun 2004 15:04:27 -0000	1.4
//++ lustre/ost/Makefile.in	1 Mar 2010 11:41:44 -0000
//@ -1,4 +1,4 @@
//MODULES := ost
//ost-objs := ost_handler.o lproc_ost.o
ost-objs := ost_handler.o lproc_ost.o ost_nrs.o
////@INCLUDE_RULES@
//ndex: lustre/ost/ost_handler.c
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/ost/ost_handler.c,v
//etrieving revision 1.184.6.35.2.1.2.19.2.3
//iff -p -u -r1.184.6.35.2.1.2.19.2.3 ost_handler.c
//-- lustre/ost/ost_handler.c	13 Jan 2009 23:54:06 -0000	1.184.6.35.2.1.2.19.2.3
//++ lustre/ost/ost_handler.c	1 Mar 2010 11:42:01 -0000
//@ -588,6 +588,7 @@ static int ost_brw_read(struct ptlrpc_re
         __u32  size[2] = { sizeof(struct ptlrpc_body), sizeof(*body) };
         int niocount, npages, nob = 0, rc, i;
         int no_reply = 0;
        struct ptlrpc_service *svc = req->rq_rqbd->rqbd_service;
         ENTRY;
//         if (OBD_FAIL_CHECK(OBD_FAIL_OST_BRW_READ_BULK))
//@ -772,6 +773,7 @@ static int ost_brw_read(struct ptlrpc_re
                 repbody = lustre_msg_buf(req->rq_repmsg, REPLY_REC_OFF,
                                          sizeof(*repbody));
                 memcpy(&repbody->oa, &body->oa, sizeof(repbody->oa));
                repbody->oa.o_padding_1 = (svc->srv_nrs->nrs_ops->nrs_get_rcc(svc->srv_nrs, body->oa.o_padding_1));
         }
//  out_lock:
//@ -822,6 +824,7 @@ static int ost_brw_write(struct ptlrpc_r
         obd_count                client_cksum = 0, server_cksum = 0;
         cksum_type_t             cksum_type = OBD_CKSUM_CRC32;
         int                      no_reply = 0;
        struct ptlrpc_service *svc = req->rq_rqbd->rqbd_service;
         ENTRY;
//         if (OBD_FAIL_CHECK(OBD_FAIL_OST_BRW_WRITE_BULK))
//@ -981,6 +984,7 @@ static int ost_brw_write(struct ptlrpc_r
                                  sizeof(*repbody));
         memcpy(&repbody->oa, &body->oa, sizeof(repbody->oa));
//        repbody->oa.o_padding_1 = (svc->srv_nrs->nrs_ops->nrs_get_rcc(svc->srv_nrs, body->oa.o_padding_1));
         if (client_cksum != 0 && rc == 0) {
                 static int cksum_counter;
////@ -1057,6 +1061,9 @@ static int ost_brw_write(struct ptlrpc_r
                        client_cksum, server_cksum, new_cksum);
         }
//        CDEBUG(D_RPCTRACE, "finish write %p buffer[%d] size %d\n",
                req->rq_reqmsg, REQ_REC_OFF + 2, (int)(niocount * sizeof(*remote_nb)));

         if (rc == 0) {
                 int nob = 0;
////@ -2007,10 +2014,16 @@ static int ost_setup(struct obd_device *
         ost->ost_io_service->srv_init = ost_thread_init;
         ost->ost_io_service->srv_done = ost_thread_done;
         ost->ost_io_service->srv_cpu_affinity = 1;
        ost->ost_service->srv_rw = ost->ost_io_service;

         rc = ptlrpc_start_threads(obd, ost->ost_io_service);
         if (rc)
                 GOTO(out_io, rc = -EINVAL);
//        rc = ost_io_nrs_init(ost->ost_io_service);
        if (rc)
                GOTO(out_io, rc);

         ping_evictor_start();
//         RETURN(0);
//@ -2108,7 +2121,8 @@ static int __init ost_init(void)
                               "dynamic thread startup\n");
                 oss_num_threads = ost_num_threads;
         }
//

        ost_register_nrs_types();
         RETURN(rc);
//}
////ndex: lustre/ost/ost_internal.h
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/ost/ost_internal.h,v
//etrieving revision 1.2.34.5.6.2.24.1
//iff -p -u -r1.2.34.5.6.2.24.1 ost_internal.h
//-- lustre/ost/ost_internal.h	9 Jan 2009 04:35:02 -0000	1.2.34.5.6.2.24.1
//++ lustre/ost/ost_internal.h	1 Mar 2010 11:42:01 -0000
//@ -87,4 +87,14 @@ enum {
         NUM_SYNC_ON_CANCEL_STATES
//};
//enum nrs_stat {
        NRS_COMPLETE_REQS = 0,
        NRS_COMPLETE_OBJECTS,
        NRS_QUEUED_REQS,
        NRS_QUEUED_OBJECTS,
        NRS_STAT_MAX,
};

void ost_register_nrs_types(void);
int ost_io_nrs_init(struct ptlrpc_service *svc);
//#endif /* OST_INTERNAL_H */
//ndex: lustre/ptlrpc/service.c
//==================================================================
//CS file: /cvsroot/cfs/lustre-core/ptlrpc/service.c,v
//etrieving revision 1.114.30.25.2.1.2.17
//iff -p -u -r1.114.30.25.2.1.2.17 service.c
//-- lustre/ptlrpc/service.c	24 Nov 2008 13:34:41 -0000	1.114.30.25.2.1.2.17
//++ lustre/ptlrpc/service.c	1 Mar 2010 11:42:15 -0000
//@ -517,6 +517,9 @@ static void ptlrpc_server_finish_request
         list_del_init(&req->rq_timed_list);
         spin_unlock(&svc->srv_at_lock);
//        if (svc->srv_nrs)
                svc->srv_nrs->nrs_ops->nrs_finish(svc->srv_nrs, req);

         ptlrpc_server_drop_request(req);
//}
////@ -959,11 +962,16 @@ static void ptlrpc_hpreq_reorder_nolock(
         LASSERT(svc != NULL);
         spin_lock(&req->rq_lock);
         if (req->rq_hp == 0) {
                struct nrs *s = svc->srv_nrs;
                 int opc = lustre_msg_get_opc(req->rq_reqmsg);
//                 /* Add to the high priority queue. */
//                list_move_tail(&req->rq_list, &svc->srv_request_hpq);
//                req->rq_hp = 1;
                if (s && s->nrs_on)
                        s->nrs_ops->nrs_priority(s, req, &svc->srv_request_hpq);
                else {
                        list_move_tail(&req->rq_list, &svc->srv_request_hpq);
                        req->rq_hp = 1;
                }
                 if (opc != OBD_PING)
                         DEBUG_REQ(D_NET, req, "high priority req");
         }
//@ -1007,12 +1015,13 @@ static int ptlrpc_server_request_add(str
                                      struct ptlrpc_request *req)
//{
         int rc;
        struct nrs *s = svc->srv_nrs;
         ENTRY;
//         rc = ptlrpc_server_hpreq_check(req);
         if (rc < 0)
                 RETURN(rc);
//

         spin_lock(&svc->srv_lock);
         /* Before inserting the request into the queue, check if it is not
          * inserted yet, or even already handled -- it may happen due to
//@ -1020,8 +1029,17 @@ static int ptlrpc_server_request_add(str
         if (req->rq_phase == RQ_PHASE_NEW && list_empty(&req->rq_list)) {
                 if (rc)
                         ptlrpc_hpreq_reorder_nolock(svc, req);
//                else
//                        list_add_tail(&req->rq_list, &svc->srv_request_queue);
                else {
                        int nrs_handled = REQ_NRS_SKIP;

                        if (s && s->nrs_on) {
                                struct nrs_ops *ops = s->nrs_ops;

                                nrs_handled = ops->nrs_enqueue(svc->srv_nrs, req);
                        }
                        if (nrs_handled != REQ_NRS_ACCEPT)
                                list_add_tail(&req->rq_list, &svc->srv_request_queue);
                }
         }
         spin_unlock(&svc->srv_lock);
////@ -1043,13 +1061,17 @@ static struct ptlrpc_request *
//ptlrpc_server_request_get(struct ptlrpc_service *svc, int force)
//{
         struct ptlrpc_request *req = NULL;
        struct nrs *s = svc->srv_nrs;
         ENTRY;
//         if (ptlrpc_server_allow_normal(svc, force) &&
//            !list_empty(&svc->srv_request_queue) &&
             (list_empty(&svc->srv_request_hpq) ||
              svc->srv_hpreq_count >= svc->srv_hpreq_ratio)) {
//                req = list_entry(svc->srv_request_queue.next,
                if (s && s->nrs_queued_num)
                        req = s->nrs_ops->nrs_dequeue(s);

                if (req == NULL && !list_empty(&svc->srv_request_queue))
                        req = list_entry(svc->srv_request_queue.next,
                                  struct ptlrpc_request, rq_list);
                 svc->srv_hpreq_count = 0;
         } else if (!list_empty(&svc->srv_request_hpq)) {
//@ -1063,7 +1085,8 @@ ptlrpc_server_request_get(struct ptlrpc_
//static int ptlrpc_server_request_pending(struct ptlrpc_service *svc, int force)
//{
         return ((ptlrpc_server_allow_normal(svc, force) &&
//                 !list_empty(&svc->srv_request_queue)) ||
                 (!list_empty(&svc->srv_request_queue) ||
                 (svc->srv_nrs && svc->srv_nrs->nrs_queued_num))) ||
                 !list_empty(&svc->srv_request_hpq));
//}
////@ -1961,6 +1984,9 @@ int ptlrpc_unregister_service(struct ptl
                 OBD_FREE(rs, service->srv_max_reply_size);
         }
//       if (service->srv_nrs)
               service->srv_nrs->nrs_destory(service);

         /* In case somebody rearmed this in the meantime */
         cfs_timer_disarm(&service->srv_at_timer);
////-- /dev/null	2010-02-28 23:25:39.678528422 +0800
//++ lustre/ost/ost_nrs.c	2010-03-01 03:06:00.000000000 +0800
//@ -0,0 +1,2222 @@
/* vim:expandtab:shiftwidth=8:tabstop=8:
 *
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
 * http://www.sun.com/software/products/lustre/docs/GPLv2.pdf
 *
 * Please contact Sun Microsystems, Inc., 4150 Network Circle, Santa Clara,
 * CA 95054 USA or visit www.sun.com if you need additional information or
 * have any questions.
 *
 * GPL HEADER END
 */
/*
 * Copyright  2008 Sun Microsystems, Inc. All rights reserved
 * Use is subject to license terms.
 */
/*
 * This file is part of Lustre, http://www.lustre.org/
 * Lustre is a trademark of Sun Microsystems, Inc.
 */

/**
 * Quantum-based Object based Round Robin NRS algorithm
 */

#define DEBUG_SUBSYSTEM S_OST
#include <linux/module.h>
#include <obd_ost.h>
#include <lustre_net.h>
#include <lustre_dlm.h>
#include <lustre_export.h>
#include <lustre_debug.h>
#include <linux/init.h>
#include <lprocfs_status.h>
#include <libcfs/list.h>
#include <lustre_quota.h>
#include <lustre_log.h>
#include "ost_internal.h"
#include <linux/rbtree.h>
#include <linux/delay.h>

#define NRS_ENQUEUE     0
#define NRS_DEQUEUE     1
#define NRS_FINISH      2
#define NRS_PRIO        3
#define NRS_NONE        4

static spinlock_t nrs_list_lock = SPIN_LOCK_UNLOCKED;
static LIST_HEAD(nrs_list);

static struct nrs_type *nrs_find(const char *name)
{
        struct nrs_type *nt;

        list_for_each_entry(nt, &nrs_list, nt_list) {
                if (!strcmp(nt->nt_name, name))
                        return nt;
        }

        return NULL;
}

static void nrs_register(struct nrs_type *nt)
{
        spin_lock(&nrs_list_lock);
        if (nrs_find(nt->nt_name) == NULL)
                list_add_tail(&nt->nt_list, &nrs_list);
        spin_unlock(&nrs_list_lock);
}

static struct nrs_type *nrs_get_type(char *name)
{
        struct nrs_type *nt;

        spin_lock(&nrs_list_lock);
        nt = nrs_find(name);
        spin_unlock(&nrs_list_lock);

        return nt;
}

/*
 * FIXME: It needs a barrier during switching.
 */
static int nrs_switch(struct nrs *s, char *name)
{
        LIST_HEAD(rpc_list);
        struct ptlrpc_request *req;
        struct nrs_ops *ops = s->nrs_ops;
        int rc;
        ENTRY;

        if (strcmp(s->nrs_type->nt_name, name) == 0)
                return 0;

        /*
         * First disable NRS and use the default scheduling path,
         * and then shift the requests from old NRS to the new one.
         */
        s->nrs_on = 0;

        while (s->nrs_queued_num > 0) {
                req = ops->nrs_dequeue(s);
                if (req == NULL) {
                        LASSERT(s->nrs_queued_num == 0);
                        break;
                }
                list_add_tail(&req->rq_list, &rpc_list);
        }

        /*
         * Ugly strategy to remove the proc entry created by the old NRS algo.
         */
        LPROCFS_EXIT();
        ops->nrs_exit(s);
        LPROCFS_ENTRY();

        s->nrs_type = nrs_get_type(name);
        s->nrs_ops = ops = &s->nrs_type->nt_ops;
        rc = ops->nrs_init(s);
        if (rc)
                RETURN(rc);

        while (!list_empty(&rpc_list)) {
                int rc;

                req = list_entry(rpc_list.next, struct ptlrpc_request, rq_list);
                list_del_init(&req->rq_list);
                rc = ops->nrs_enqueue(s, req);
                if (rc == REQ_NRS_SKIP || rc < 0) {
                        struct ptlrpc_service *svc = s->nrs_svc;

                        spin_lock(s->nrs_lock);
                        list_add_tail(&req->rq_list, &svc->srv_request_queue);
                        spin_unlock(s->nrs_lock);
                }
        }

        s->nrs_on = 1;

        RETURN(0);
}

#ifdef LPROCFS
static int nrs_rd_queue_depth(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct nrs *nrs = data;
        int rc;

        rc = snprintf(page, count, "%d\n", nrs->nrs_queued_num );
        return rc;
}

static int nrs_rd_max_depth(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct nrs *nrs = data;
        int rc;

        rc = snprintf(page, count, "%d\n", nrs->nrs_max_depth);
        return rc;
}

static int nrs_rd_version(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        int rc;

        rc = snprintf(page, count, "3.0\n");
        return rc;
}

static int nrs_wr_max_depth(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct nrs *nrs = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val < 0)
                return -EINVAL;

        nrs->nrs_max_depth = val;
        return count;
}

static int nrs_rd_max_serv_time(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct nrs *nrs = data;
        int rc;

        rc = snprintf(page, count, "%ldus\n", nrs->nrs_max_serv_time);
        return rc;
}

static int nrs_wr_max_serv_time(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct nrs *nrs = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val < 0)
                return -EINVAL;

        nrs->nrs_max_serv_time = (long)val;
        return count;
}

static int nrs_rd_sched_on(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct nrs *nrs = data;
        int rc;

        rc = snprintf(page, count, "%d\n", nrs->nrs_on);
        return rc;
}

static int nrs_wr_sched_on(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct nrs *nrs = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val != 0 && val != 1)
                return -EINVAL;

        nrs->nrs_on = val;
        return count;
}

static int nrs_wr_stat_cleanup(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct nrs *nrs = data;

        nrs->nrs_max_depth = 0;
        nrs->nrs_max_serv_time = 0;
        nrs->nrs_ops->nrs_stat_cleanup(nrs);

        return count;
}

static int nrs_rd_algo(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct nrs *s = data;
        struct nrs_type *nt;
        int len = 0;

        spin_lock(&nrs_list_lock);
        list_for_each_entry(nt, &nrs_list, nt_list) {
                if (!strcmp(s->nrs_type->nt_name, nt->nt_name))
                        len += snprintf(page + len,
                                count - len, "[%s] ", nt->nt_name);
                else
                        len += snprintf(page + len,
                                count - len, "%s ", nt->nt_name);
        }
        spin_unlock(&nrs_list_lock);

        len += snprintf(page + len, count - len, "\n");
        return len;
}

static int nrs_wr_algo(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct nrs *s = data;
        char *p, name[NRS_NAME_MAX];
        int len, rc;

        name[sizeof(name) - 1] = '\0';
        strncpy(name, buffer, sizeof(name) - 1);
        p = strchr(name, '\n');
        *p = '\0';
        len = strlen(name);

        if (len && name[len - 1] == '\n')
                name[len - 1] = '\0';

        if (nrs_find(name) == NULL)
                return -EINVAL;

        rc = nrs_switch(s, name);
        if (rc) {
                CERROR("Failed to change the NRS algorithm.\n");
                return rc;
        }
        return count;
}

static struct lprocfs_vars lprocfs_nrs_vars[] = {
        { "rpc_queue_depth", nrs_rd_queue_depth, 0, 0 },
        { "max_queue_depth", nrs_rd_max_depth, nrs_wr_max_depth, 0 },
        { "max_serv_time", nrs_rd_max_serv_time, nrs_wr_max_serv_time, 0 },
        { "sched_on", nrs_rd_sched_on, nrs_wr_sched_on, 0 },
        { "cleanup", 0, nrs_wr_stat_cleanup, 0 },
        { "algo", nrs_rd_algo, nrs_wr_algo, 0 },
        { "version", nrs_rd_version, 0, 0 },
        { 0 }
};

void lprocfs_nrs_stats_inc(struct nrs *nrs, enum nrs_stat which)
{
        LASSERTF(which >= 0 && which < NRS_STAT_MAX, "which: %u\n", which);
        lprocfs_counter_incr(nrs->nrs_stats, which);
}

void lprocfs_nrs_stats_dec(struct nrs *nrs, enum nrs_stat which)
{
        LASSERTF(which >= 0 && which < NRS_STAT_MAX, "which: %u\n", which);
        lprocfs_counter_decr(nrs->nrs_stats, which);
}

static const char *nrs_counter_name[] = {
        [NRS_COMPLETE_REQS] = "complete_reqs",
        [NRS_COMPLETE_OBJECTS] = "complete_ objs ",
        [NRS_QUEUED_REQS] = "queued_reqs",
        [NRS_QUEUED_OBJECTS] = "queued_objs ",
};

static void nrs_procfs_fini(struct nrs *nrs)
{
        if (nrs->nrs_stats)
                lprocfs_free_stats(&nrs->nrs_stats);

        if (nrs->nrs_proc_entry) {
                lprocfs_remove(&nrs->nrs_proc_entry);
                nrs->nrs_proc_entry = NULL;
        }
}

static int nrs_procfs_init(struct ptlrpc_service *svc)
{
        struct nrs *nrs;
        int rc, i;
        ENTRY;

        if (svc->srv_procroot == NULL)
                return 0;

        nrs = svc->srv_nrs;
        nrs->nrs_proc_entry = lprocfs_register("nrs", svc->srv_procroot,
                                lprocfs_nrs_vars, nrs);
        if (IS_ERR(nrs->nrs_proc_entry)) {
                rc = PTR_ERR(nrs->nrs_proc_entry);
                CERROR("Error %d setting up lprocfs for nrs\n", rc);
                nrs->nrs_proc_entry = NULL;
                GOTO(out, rc);
        }

        nrs->nrs_stats = lprocfs_alloc_stats(ARRAY_SIZE(nrs_counter_name),
                                           LPROCFS_STATS_FLAG_PERCPU);
        if (nrs->nrs_stats == NULL)
                GOTO(out, rc = -ENOMEM);

        for (i = 0; i < ARRAY_SIZE(nrs_counter_name); i++)
                lprocfs_counter_init(nrs->nrs_stats, i, 0,
                                     nrs_counter_name[i], "counts");

        rc = lprocfs_register_stats(nrs->nrs_proc_entry, "nrs_stats",
                                    nrs->nrs_stats);
out:
        if (rc)
                nrs_procfs_fini(nrs);

        RETURN(rc);
}

#else
static inline void lprocfs_nrs_stats_inc(struct nrs *nrs, enum nrs_stat which) {return;}
static inline void lprocfs_nrs_stats_dec(struct nrs *nrs, enum nrs_stat which) {return;}
static inline int  nrs_procfs_init(struct ptlrpc_service *svc) {return 0;}
static inline void nrs_procfs_fini(struct nrs *nrs) {return;}
#endif

static void nrs_destory(struct ptlrpc_service *svc)
{
        struct nrs *s = svc->srv_nrs;

        LASSERT(s != NULL);

        s->nrs_ops->nrs_exit(s);
        nrs_procfs_fini(s);
        OBD_FREE_PTR(s);
        svc->srv_nrs_type = NULL;
        svc->srv_nrs = NULL;
}

static int nrs_attach(struct ptlrpc_service *svc, char *name)
{
        struct nrs *s;
        int rc;
        ENTRY;

        OBD_ALLOC_PTR(s);
        if (!s)
                RETURN(-ENOMEM);

        s->nrs_lock = &svc->srv_lock;
        s->nrs_queued_num = 0;
        s->nrs_destory = nrs_destory;
        s->nrs_svc = svc;
        svc->srv_nrs = s;

        rc = nrs_procfs_init(svc);
        if (rc)
              GOTO(out, rc);

        s->nrs_type = nrs_get_type(name);
        s->nrs_ops = &s->nrs_type->nt_ops;
        rc = s->nrs_ops->nrs_init(s);
out:
        if (rc)
               s->nrs_destory(svc);

        s->nrs_on = 1;
        RETURN(rc);
}

static void nrs_parse_ioreq(struct ptlrpc_request *req, int phase)
{
        struct niobuf_remote *nb;
        struct ost_body *body;
        struct obd_ioobj *ioo;
        int objcount, niocount;
        int i;

        LASSERTF(req->rq_nrs_node.pnn_start == 0 && req->rq_nrs_node.pnn_end == 0,
                "Req start: "LPU64" end: "LPU64" is not zero!",
                req->rq_nrs_node.pnn_start, req->rq_nrs_node.pnn_end);

        body = lustre_swab_reqbuf(req, REQ_REC_OFF, sizeof(*body),
                        lustre_swab_obdo);
        LASSERT(body != NULL);

        objcount = lustre_msg_buflen(req->rq_reqmsg, REQ_REC_OFF + 1) /
                        sizeof(*ioo);

        ioo = lustre_msg_buf(req->rq_reqmsg, REQ_REC_OFF + 1,
                      objcount * sizeof(*ioo));
        LASSERT(ioo != NULL);

        for (niocount = i = 0; i < objcount; i++)
                niocount += ioo[i].ioo_bufcnt;

        nb = lustre_msg_buf(req->rq_reqmsg, REQ_REC_OFF + 2,
                niocount * sizeof(*nb));
        LASSERT(nb != NULL);

        req->rq_nrs_node.pnn_obj = ioo;
        req->rq_nrs_node.pnn_start = nb[0].offset & CFS_PAGE_MASK;
        req->rq_nrs_node.pnn_end = (nb[ioo->ioo_bufcnt - 1].offset +
                nb[ioo->ioo_bufcnt - 1].len - 1) | ~CFS_PAGE_MASK;

        CDEBUG(D_RPCTRACE, "%s an I/O request xid "LPU64
                " from %s objid %llu Extent ["
                LPU64" -- "LPU64"], current queued number: %d\n",
                phase == NRS_ENQUEUE ? "Enqueue" : "Dequeue", req->rq_xid,
                libcfs_id2str(req->rq_peer),
                req->rq_nrs_node.pnn_obj->ioo_id,
                req->rq_nrs_node.pnn_start,
                req->rq_nrs_node.pnn_end,
                req->rq_rqbd->rqbd_service->srv_nrs->nrs_queued_num);
}

static inline void nrs_trace_ioreq(struct nrs *s,
                struct ptlrpc_request *req, int phase)
{
        struct niobuf_remote *nb;
        struct ost_body *body;
        struct obd_ioobj *ioo;
        int objcount, niocount;
        int i;

        /* Just for Debug ... */
        body = lustre_swab_reqbuf(req, REQ_REC_OFF, sizeof(*body),
                        lustre_swab_obdo);

        LASSERT(body != NULL);
        objcount = lustre_msg_buflen(req->rq_reqmsg, REQ_REC_OFF + 1) /
                        sizeof(*ioo);

        ioo = lustre_msg_buf(req->rq_reqmsg, REQ_REC_OFF + 1,
                      objcount * sizeof(*ioo));

        LASSERT(ioo != NULL);

        for (niocount = i = 0; i < objcount; i++)
                niocount += ioo[i].ioo_bufcnt;

        nb = lustre_msg_buf(req->rq_reqmsg, REQ_REC_OFF + 2,
                niocount * sizeof(*nb));

        LASSERTF(nb != NULL, "failed in phase %d\n", phase);
        //if (phase != NRS_FINISH)
        CDEBUG(D_RPCTRACE, "%s an I/O request xid "LPU64
                " from %s objid %llu Extent ["
                LPU64" -- "LPU64"], current queued number: %d\n",
                phase == NRS_ENQUEUE ? "Enqueue" : "Dequeue", req->rq_xid,
                libcfs_id2str(req->rq_peer),
                req->rq_nrs_node.pnn_obj->ioo_id,
                req->rq_nrs_node.pnn_start,
                req->rq_nrs_node.pnn_end,
                s->nrs_queued_num);
        /*DEBUG_REQ(D_RPCTRACE, req, "%s ",
                phase == NRS_ENQUEUE ? "Enqueue" : "Dequeue");

        CDEBUG(D_RPCTRACE, "msg %p buffer[%d] (required %d, opc=%d)\n",
                req->rq_reqmsg, REQ_REC_OFF + 2, (int)(niocount * sizeof(*nb)),
                lustre_msg_get_opc(req->rq_reqmsg));*/
}

static int nrs_empty(struct nrs *s)
{
        return s->nrs_queued_num == 0;
}

/**
 * Object based Round Robin NRS algorithm.
 * There are 2 queues for Round Robin NRS:
 *  1. hash_list(rr_obj_hash) for inserting the incoming request. Each item in
 *     the hash table is the object. when read/write reqs arrive, NRS will search
 *     the hash table with the object id, create one if there are no such object,
 *     then the req will be inserted into the read or write queue of this object
 *     according to the offset. These 2 queues for the object is structured as a
 *     rbtree. Finally the read or write queue will be insert to the global
 *     work_queue.
 *
 *  2. work_queue(rr_workq) used for getting queued request and handling it. Each
 *     item in the queue are the read/write queue for the object (see 1). It maintains
 *     the queue as FCFS order, but seach item will be assigned to a quatum(the req
 *     count it can serve each time), once the quatum is used up, it will be added
 *     to the tail of the list and waiting for next schedule.
 */

#define QUANTUM_PER_ROUND 8
#define OBJ_QUEUE_QUEUED  1

#define OBRR_READ    0
#define OBRR_WRITE   1

struct data_object;
struct object_queue {
        int                 rw;
        int                 flags;
        struct list_head    list; /* list chained into global fifo rr queue */
        int                 quantum;
        obd_count           num;
        //spinlock_t          lock;
        obd_off             pos; /* the position of last serviced request */
        int                 seqnum;
        struct rb_root      root;
        struct data_object *obj;
};

#define data_obj_id     obd_id
#define data_obj_nid    lnet_nid_t
struct data_obj_key {
        data_obj_id       oid;
        data_obj_nid      nid;
};

struct data_object {
        int                   flags;
        struct data_obj_key   key;
        struct hlist_node     list;
        atomic_t              ref;
        struct object_queue   q[2];
};

#define OBRR_OBJ_HTABLE_SIZE    (1024 * 4)
struct obrr_data {
        int                   rr_quantum;
        obd_count             rr_cnt; /* number of pending requests in NRS */
        lustre_hash_t        *rr_obj_hash;
        spinlock_t            rr_lock;

        struct list_head      rr_queue;  /* global FIFO round robin queue */
        struct list_head      rr_dyn_dl; /* dynamical deadline FIFO queue */
        struct list_head      rr_mdy_dl; /* mandatory dadline FIFO queue */
        struct object_queue  *rr_workq; /* work queue */
        obd_count             rr_pending_io; /* pending I/O amount */
        obd_count             rr_max_pending_io; /* max pending I/O amount */
        obd_count             rr_queue_num; /* number of object queues with pending I/O */
        obd_count             rr_max_queue_num;

        struct obd_histogram  rr_brw_hist[2];
        cfs_proc_dir_entry_t *rr_proc_entry;
};

static void object_destory(struct data_object *obj)
{
        int i;

        LASSERTF(atomic_read(&obj->ref) == 0,
                "ref %d != 0\n", atomic_read(&obj->ref));
        for (i = 0; i < 2; i++) {
                struct object_queue *q = &obj->q[i];

                LASSERT(list_empty(&q->list));
        }

        OBD_FREE_PTR(obj);
}

#define object_get(obj)                 \
do {                                    \
        atomic_inc(&obj->ref);          \
} while (0)

#define object_put(obj)                 \
do {                                    \
        atomic_dec(&obj->ref);          \
} while (0)

/**
 * obrr object hash function
 */
static unsigned
obrr_obj_hash(lustre_hash_t *lh,  void *key, unsigned mask)
{
        struct data_obj_key  *nkey = (struct data_obj_key *)key;

        return lh_u64_hash(nkey->oid, mask);
}

static void *
obrr_obj_key(struct hlist_node *hnode)
{
        struct data_object *obj;

        obj = hlist_entry(hnode, struct data_object, list);

        RETURN(&obj->key);
}

static int
obrr_obj_compare(void *k, struct hlist_node *hnode)
{
        struct data_object *obj;
        struct data_obj_key *key;

        LASSERT(k);
        key = (struct data_obj_key *)k;

        obj = hlist_entry(hnode, struct data_object, list);

        RETURN(obj->key.nid == key->nid &&
               obj->key.oid == key->oid);
}

static void *
obrr_obj_get(struct hlist_node *hnode)
{
        struct data_object *obj;

        obj = hlist_entry(hnode, struct data_object, list);
        object_get(obj);
        RETURN(obj);
}

static void *
obrr_obj_put(struct hlist_node *hnode)
{
        struct data_object *obj;

        obj = hlist_entry(hnode, struct data_object, list);
        object_put(obj);
        RETURN(obj);
}

static lustre_hash_ops_t obj_hash_ops = {
        .lh_hash    = obrr_obj_hash,
        .lh_key     = obrr_obj_key,
        .lh_compare = obrr_obj_compare,
        .lh_get     = obrr_obj_get,
        .lh_put     = obrr_obj_put,
};

/**
 * Create a nrs object
 */
static struct data_object*
obrr_object_create(data_obj_id oid, data_obj_nid nid)
{
        struct data_object* obj;
        int i;

        OBD_ALLOC_PTR(obj);
        if (obj == NULL)
                return NULL;

        obj->key.oid = oid;
        obj->key.nid = nid;
        INIT_HLIST_NODE(&obj->list);
        atomic_set(&obj->ref, 1);

        for (i = 0; i < 2; i++) {
                struct object_queue *q = &obj->q[i];

                q->rw = i;
                //spin_lock_init(&q->lock);
                INIT_LIST_HEAD(&q->list);
                q->root = RB_ROOT;
                q->obj = obj;
        }
        return obj;
}

/**
 * lookup a object in the hash table
 */
static struct data_object*
obrr_object_lookup(struct obrr_data *d, data_obj_id oid, data_obj_nid nid)
{
        struct data_obj_key key = {.oid = oid, .nid = nid};

        RETURN(lustre_hash_lookup(d->rr_obj_hash, &key));
}

/**
 * insert a object in the hash table
 */
static int obrr_object_insert(struct obrr_data *d, struct data_object *obj)
{
        CDEBUG(D_CACHE, "insert object id "LPU64" nid "LPU64"\n",
               obj->key.oid, obj->key.nid);
        RETURN(lustre_hash_add_unique(d->rr_obj_hash, &obj->key, &obj->list));
}

/**
 * Compare two node of the rbtree, which is used when insert a req into
 * the object queue.
 */
int obrr_compare(struct ptlrpc_nrs_node *one, struct ptlrpc_nrs_node *two)
{
        if (one->pnn_start < two->pnn_start)
                return -1;
        if (one->pnn_start > two->pnn_start)
                return 1;

        if (one->pnn_end < two->pnn_end)
                return -1;

        if (one->pnn_end > two->pnn_end)
                return 1;
        return 0; /* they are the same object and overlap */
}

/**
 * Insert the node (req attached) to the rbtree.
 */
static void __obrr_queue_insert(struct rb_root *root, struct ptlrpc_nrs_node *node)
{
        struct ptlrpc_nrs_node *walk;
        struct rb_node **p, *parent;

        p = &root->rb_node;
        parent = NULL;
        while (*p) {
                parent = *p;
                walk = rb_entry(parent, struct ptlrpc_nrs_node, pnn_node);
                if (obrr_compare(node, walk) >= 0)
                        p = &(*p)->rb_right;
                else
                        p = &(*p)->rb_left;
        }
        rb_link_node(&node->pnn_node, parent, p);
        rb_insert_color(&node->pnn_node, root);
}

/**
 * Insert the request to the read/write queue for the object.
 */
static void obrr_queue_insert_req(struct nrs *s, struct ptlrpc_request *req,
                        struct object_queue *q)
{
        struct obrr_data *d = (struct obrr_data *)s->nrs_data;

        __obrr_queue_insert(&q->root, &req->rq_nrs_node);
        q->num++;

        req->rq_nrs = 1;
        req->rq_nrs_node.pnn_data = q;
        lprocfs_nrs_stats_inc(s, NRS_QUEUED_REQS);

        s->nrs_queued_num++;
        s->nrs_max_depth = max(s->nrs_max_depth, s->nrs_queued_num);
        d->rr_pending_io += req->rq_nrs_node.pnn_end -
                        req->rq_nrs_node.pnn_start + 1;
        d->rr_max_pending_io = max(d->rr_max_pending_io, d->rr_pending_io);

        if (!(q->flags & OBJ_QUEUE_QUEUED)) {
                q->flags |= OBJ_QUEUE_QUEUED;
                list_add_tail(&q->list, &d->rr_queue);
                d->rr_queue_num++;
                d->rr_max_queue_num = max(d->rr_max_queue_num, d->rr_queue_num);
        }

        /* TODO: add to deadline queue */
}

static int obrr_enqueue(struct nrs *s, struct ptlrpc_request *req)
{
        struct obrr_data *d = (struct obrr_data *)s->nrs_data;
        struct data_object *obj;
        struct obd_ioobj *ioo;
        int i, opc, rc = 0;
        ENTRY;

        if (!req->rq_export)
                RETURN(-EINVAL);

        opc = lustre_msg_get_opc(req->rq_reqmsg);
        if (opc != OST_READ && opc != OST_WRITE)
                RETURN(REQ_NRS_SKIP);

        nrs_parse_ioreq(req, NRS_ENQUEUE);
        ioo = req->rq_nrs_node.pnn_obj;
repeat:
        obj = obrr_object_lookup(d, ioo->ioo_id, req->rq_peer.nid);
        if (!obj) {
                obj = obrr_object_create(ioo->ioo_id, req->rq_peer.nid);
                if (!obj)
                        RETURN(-ENOMEM);

                rc = obrr_object_insert(d, obj);
                if (rc == -EALREADY) {
                        LASSERT(atomic_read(&obj->ref) == 1);
                        object_put(obj);
                        object_destory(obj);
                        GOTO(repeat, rc);
                }
                lprocfs_nrs_stats_inc(s, NRS_QUEUED_OBJECTS);
        }

        i = (opc == OST_READ) ? 0 : 1;
        obrr_queue_insert_req(s, req, &obj->q[i]);

        nrs_trace_ioreq(s, req, NRS_ENQUEUE);
        RETURN(REQ_NRS_ACCEPT);
}

/**
 * Try to remove the object from the hash table
 */
static void obrr_object_release(struct nrs *s, struct data_object *obj)
{
        if (atomic_dec_return(&obj->ref) == 1) {
                struct obrr_data *d = s->nrs_data;
                lustre_hash_t *lh = d->rr_obj_hash;
                lustre_hash_bucket_t *lhb;
                unsigned i;
                void *_obj;


                CDEBUG(D_CACHE, "Try to release object id "LPU64" nid "LPU64"ref%d\n",
                       obj->key.oid, obj->key.nid, (int)atomic_read(&obj->ref));

                lprocfs_nrs_stats_inc(s, NRS_COMPLETE_OBJECTS);
                lprocfs_nrs_stats_dec(s, NRS_QUEUED_OBJECTS);
                read_lock(&lh->lh_rwlock);
                i = lh_hash(lh, &obj->key, lh->lh_cur_mask);
                lhb = &lh->lh_buckets[i];
                LASSERT(i <= lh->lh_cur_mask);
                LASSERT(!hlist_unhashed(&obj->list));

                write_lock(&lhb->lhb_rwlock);
                if (atomic_read(&obj->ref) == 1) {
                        _obj = __lustre_hash_bucket_del(lh, lhb, &obj->list);
                        LASSERTF(_obj == obj && atomic_read(&obj->ref) == 0,
                                "ref %d != 0\n", atomic_read(&obj->ref));
                        object_destory(obj);
                }
                write_unlock(&lhb->lhb_rwlock);
                read_unlock(&lh->lh_rwlock);
        }
}

/* TODO: the elevator scheduling algorithm should choose the request according
 * to the position of the last serviced request.
 * Already hold srv_lock.
 */
static struct ptlrpc_request *obrr_dequeue_req(struct nrs *s)
{

        struct ptlrpc_nrs_node *node = NULL;
        struct rb_node *rbnode;
        struct ptlrpc_request *req;
        struct obrr_data *d = (struct obrr_data *)s->nrs_data;
        struct object_queue *wq;
        obd_count seq1 = 0, seq2 = 0;

repeat:
        if (!d->rr_workq) {
                if (list_empty(&d->rr_queue))
                        RETURN(NULL);

                d->rr_workq = list_entry(d->rr_queue.next, struct object_queue, list);
                list_del_init(&d->rr_workq->list);
                d->rr_workq->quantum = d->rr_quantum;
                d->rr_queue_num--;
        }
        wq = d->rr_workq;
        object_get(wq->obj);

        if (wq->num == 0) {
                LASSERT(wq != d->rr_workq);
                obrr_object_release(s, wq->obj);
                goto repeat;
        }
        LASSERT(wq->num > 0);
        for ( rbnode = wq->root.rb_node; rbnode != NULL;
              rbnode = rbnode->rb_left) {
                if (rbnode->rb_left == NULL) {
                        node = rb_entry(rbnode, struct ptlrpc_nrs_node,
                                        pnn_node);
                        rb_erase(rbnode, &wq->root);
                        break;
                }
        }

        wq->quantum--;
        wq->num--;
        if (wq->seqnum == 0 || (wq->pos + 1 == node->pnn_start))
                wq->seqnum++;
        else {
                seq1 = wq->seqnum;
                wq->seqnum = 1;
        }
        wq->pos = node->pnn_end;

        s->nrs_queued_num--;
        LASSERT(s->nrs_queued_num >= 0);
        if (wq->num == 0) {
                d->rr_workq = NULL;
                wq->flags &= ~OBJ_QUEUE_QUEUED;
        } else if (wq->quantum == 0) { /* use out quantum */
                d->rr_workq = NULL;
                d->rr_queue_num++;
                list_add_tail(&wq->list, &d->rr_queue);
        }

        if (wq->num == 0 || wq->quantum == 0) {
                if (seq1 == 0)
                        seq1 = wq->seqnum;
                else {
                        seq2 = wq->seqnum;
                        LASSERT(seq2 == 1);
                }
                wq->seqnum = 0;
        }
        req = container_of(node, struct ptlrpc_request, rq_nrs_node);
        LASSERT(req->rq_nrs == 1 && req->rq_nrs_node.pnn_data != NULL);
        req->rq_nrs = 0;
        req->rq_nrs_node.pnn_data = NULL;
        object_put(wq->obj);

        if (seq1)
                lprocfs_oh_tally(&d->rr_brw_hist[wq->rw], seq1);
        if (seq2)
                lprocfs_oh_tally(&d->rr_brw_hist[wq->rw], seq2);

        lprocfs_nrs_stats_inc(s, NRS_COMPLETE_REQS);
        lprocfs_nrs_stats_dec(s, NRS_QUEUED_REQS);
        CDEBUG(D_CACHE, "dequeue a request xid "LPU64" ["LPU64", "LPU64"] \n",
               req->rq_xid, node->pnn_start, node->pnn_end);
        /**
         * data object is refered when a incoming request enqueued to
         * its object queue; derefer and try to unhash and release
         * data object when a requese is dequeued.
         */
        obrr_object_release(s, wq->obj);

        nrs_trace_ioreq(s, req, NRS_DEQUEUE);
        RETURN(req);
}

/**
 * dequeue a request from the workqueue
 */
static struct ptlrpc_request *obrr_dequeue(struct nrs *s)
{
        return (obrr_dequeue_req(s));
}

static void obrr_finish(struct nrs *s, struct ptlrpc_request *req)
{
        struct obrr_data *d = (struct obrr_data *) s->nrs_data;
        struct timeval now;
        int opc;

        if (req->rq_phase != RQ_PHASE_COMPLETE)
                return;

        do_gettimeofday(&now);
        spin_lock(&d->rr_lock);
        s->nrs_max_serv_time = max(s->nrs_max_serv_time,
                cfs_timeval_sub(&now, &req->rq_arrival_time, NULL));

        opc = lustre_msg_get_opc(req->rq_reqmsg);
        if (opc == OST_READ || opc == OST_WRITE)
                d->rr_pending_io -= (req->rq_nrs_node.pnn_end -
                                     req->rq_nrs_node.pnn_start + 1);
        spin_unlock(&d->rr_lock);
}

static void obrr_stat_cleanup(struct nrs *s)
{
        struct obrr_data *d = s->nrs_data;
        int i;

        d->rr_max_pending_io = 0;
        d->rr_max_queue_num = 0;

        for (i = 0; i < 2; i++)
                lprocfs_oh_clear(&d->rr_brw_hist[i]);
}

#ifdef LPROCFS
static int lprocfs_obrr_rd_pending_io(char *page, char **start, off_t off, int count,
                int *eof, void *data)
{
        struct obrr_data *d = data;
        int rc;

        rc = snprintf(page, count, "%u bytes\n", d->rr_pending_io);
        return rc;
}

static int lprocfs_obrr_rd_max_pending_io(char *page, char **start, off_t off, int count,
                int *eof, void *data)
{
        struct obrr_data *d = data;
        int rc;

        rc = snprintf(page, count, "%u bytes\n", d->rr_max_pending_io);
        return rc;
}

static int lprocfs_obrr_wr_max_pending_io(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct obrr_data *d = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;
        if (val < 0)
                return -EINVAL;

        d->rr_max_pending_io = val;
        return count;
}

static int lprocfs_obrr_rd_queue_num(char *page, char **start, off_t off, int count,
                int *eof, void *data)
{
        struct obrr_data *d = data;
        int rc;

        rc = snprintf(page, count, "%u\n", d->rr_queue_num);
        return rc;
}

static int lprocfs_obrr_rd_max_queue_num(char *page, char **start, off_t off, int count,
                int *eof, void *data)
{
        struct obrr_data *d = data;
        int rc;

        rc = snprintf(page, count, "%u\n", d->rr_max_queue_num);
        return rc;
}

static int lprocfs_obrr_wr_max_queue_num(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct obrr_data *d = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;
        if (val < 0)
                return -EINVAL;

        d->rr_max_queue_num = val;
        return count;
}

static int lprocfs_obrr_rd_quantum(char *page, char **start, off_t off, int count,
                int *eof, void *data)
{
        struct obrr_data *d = data;
        int rc;

        rc = snprintf(page, count, "%d\n", d->rr_quantum);
        return rc;
}

static int lprocfs_obrr_wr_quantum(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct obrr_data *d = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;
        if (val < 0)
                return -EINVAL;

        d->rr_quantum = val;
        return count;
}

struct lprocfs_vars lprocfs_obrr_vars[] = {
        { "cur_pending_io", lprocfs_obrr_rd_pending_io, 0, 0 },
        { "max_pending_io", lprocfs_obrr_rd_max_pending_io, lprocfs_obrr_wr_max_pending_io, 0 },
        { "queue_num", lprocfs_obrr_rd_queue_num, 0, 0 },
        { "max_queue_num", lprocfs_obrr_rd_max_queue_num, lprocfs_obrr_wr_max_queue_num, 0 },
        { "quantum", lprocfs_obrr_rd_quantum, lprocfs_obrr_wr_quantum, 0 },
        { 0 }
};

#define pct(a,b) (b ? a * 100 / b : 0)

static void display_brw_stats(struct seq_file *seq, char *name, char *units,
        struct obd_histogram *read, struct obd_histogram *write, int log2)
{
        unsigned long read_tot, write_tot, r, w, read_cum = 0, write_cum = 0;
        int i;

        seq_printf(seq, "\n%26s read      |     write\n", " ");
        seq_printf(seq, "%-22s %-5s %% cum %% |  %-5s %% cum %%\n",
                   name, units, units);

        read_tot = lprocfs_oh_sum(read);
        write_tot = lprocfs_oh_sum(write);
        for (i = 0; i < OBD_HIST_MAX; i++) {
                r = read->oh_buckets[i];
                w = write->oh_buckets[i];
                read_cum += r;
                write_cum += w;
                if (read_cum == 0 && write_cum == 0)
                        continue;

                if (!log2)
                        seq_printf(seq, "%u", i);
                else if (i < 10)
                        seq_printf(seq, "%u", 1<<i);
                else if (i < 20)
                        seq_printf(seq, "%uK", 1<<(i-10));
                else
                        seq_printf(seq, "%uM", 1<<(i-20));

                seq_printf(seq, ":\t\t%10lu %3lu %3lu   | %4lu %3lu %3lu\n",
                           r, pct(r, read_tot), pct(read_cum, read_tot),
                           w, pct(w, write_tot), pct(write_cum, write_tot));

                if (read_cum == read_tot && write_cum == write_tot)
                        break;
        }
}

static int obrr_brw_stats_seq_show(struct seq_file *seq, void *v)
{
        struct nrs *s = seq->private;
        struct obrr_data *d = s->nrs_data;
        struct timeval now;

        seq_printf(seq, "quantum: %d\n"
                        "max service time: %ldus\n"
                        "max rpc queue depth: %u\n"
                        "max pending IO: %u\n"
                        "max object queue number: %u\n",
                        d->rr_quantum, s->nrs_max_serv_time,
                        s->nrs_max_depth, d->rr_max_pending_io,
                        d->rr_max_queue_num);

        do_gettimeofday(&now);
        seq_printf(seq, "snapshot_time:         %lu.%lu (secs.usecs)\n",
                   now.tv_sec, now.tv_usec);

        display_brw_stats(seq, "continuous requests", "rpcs",
                        &d->rr_brw_hist[0], &d->rr_brw_hist[1], 0);

        return 0;
}

static ssize_t obrr_brw_stats_seq_write(struct file *file, const char *buf,
                        size_t len, loff_t *off)
{
        struct seq_file *seq = file->private_data;
        struct nrs *s = seq->private;
        struct obrr_data *d = s->nrs_data;
        int i;

        for (i = 0; i < 2; i++)
                lprocfs_oh_clear(&d->rr_brw_hist[i]);

        return len;
}

LPROC_SEQ_FOPS(obrr_brw_stats);

static int obrr_procfs_init(struct nrs *s, struct obrr_data *d)
{
        int rc = 0;

        d->rr_proc_entry = lprocfs_register("obrr", s->nrs_proc_entry,
                                lprocfs_obrr_vars, d);
        if (IS_ERR(d->rr_proc_entry)) {
                rc = PTR_ERR(d->rr_proc_entry);
                CERROR("Error %d setting up lprocfs for obrr\n", rc);
                d->rr_proc_entry = NULL;
                return rc;
        }
        rc = lprocfs_seq_create(d->rr_proc_entry, "brw_stats",
                        0444, &obrr_brw_stats_fops, s);
        return rc;
}

static void obrr_procfs_fini(struct obrr_data *d)
{
        if (d->rr_proc_entry) {
                lprocfs_remove(&d->rr_proc_entry);
                d->rr_proc_entry = NULL;
        }
}
#else
static int obrr_procfs_init(struct nrs *s, struct obrr_data *d) { return 0; }
static void obrr_procfs_fini(struct obrr_data *d) { return; }
#endif

static void obrr_exit(struct nrs *s)
{
        struct obrr_data *data;

        if (!s)
                return;

        data = (struct obrr_data *) s->nrs_data;
        if (data) {
                obrr_procfs_fini(data);
                lustre_hash_exit(data->rr_obj_hash);
                OBD_FREE_PTR(data);
        }
        s->nrs_data = NULL;
}

static int obrr_init(struct nrs *s)
{
        struct obrr_data *d = NULL;
        int rc;
        ENTRY;

        OBD_ALLOC_PTR(d);
        if (!d)
              GOTO(out, rc = -ENOMEM);

        d->rr_obj_hash = lustre_hash_init("DATA_OBJ_HASH",
                16, 16, &obj_hash_ops, 0);
        if (d->rr_obj_hash == NULL)
                GOTO(out, rc = -ENOMEM);

        INIT_LIST_HEAD(&d->rr_queue);
        d->rr_workq = NULL;
        d->rr_quantum = QUANTUM_PER_ROUND;
        spin_lock_init(&d->rr_brw_hist[OBRR_READ].oh_lock);
        spin_lock_init(&d->rr_brw_hist[OBRR_WRITE].oh_lock);
        spin_lock_init(&d->rr_lock);

        rc = obrr_procfs_init(s, d);
out:
        if (rc)
                obrr_exit(s);

        s->nrs_data = d;
        RETURN(0);
}

static void obrr_priority(struct nrs *nrs, struct ptlrpc_request *req,
                          struct list_head *priority_list)
{
        if (req->rq_nrs == 1) {
                int opc;
                struct object_queue *queue =
                        (struct object_queue *)req->rq_nrs_node.pnn_data;
                if (queue != NULL) {
                        if (req->rq_nrs == 1) {
                                struct obrr_data *data = nrs->nrs_data;

                                rb_erase(&req->rq_nrs_node.pnn_node, &queue->root);
                                queue->num --;
                                nrs->nrs_queued_num--;
                                if (queue->num == 0) {
                                        queue->flags &= ~OBJ_QUEUE_QUEUED;
                                        if (queue == data->rr_workq)
                                                data->rr_workq = NULL;
                                        else {
                                                data->rr_queue_num--;
                                                list_del_init(&queue->list);
                                        }
                                }


                                req->rq_nrs = 0;
                                req->rq_nrs_node.pnn_data = NULL;
                        }
                        obrr_object_release(nrs, queue->obj);
                        opc = lustre_msg_get_opc(req->rq_reqmsg);
                        if (opc == OST_READ || opc == OST_WRITE)
                                nrs_trace_ioreq(nrs, req, NRS_PRIO);
                } else {
                        CWARN("req %p is being dequeued! \n", req);
                }
        }
        list_move_tail(&req->rq_list, priority_list);
        req->rq_hp = 1;
}

static int obrr_get_rcc(struct nrs *s, int nr)
{
        return 0;
}

static int obrr_update_clnr(struct nrs *s, struct ptlrpc_request *req)
{
        return 0;
}

static struct nrs_type nrs_obrr = {
        .nt_ops = {
                .nrs_enqueue = obrr_enqueue,
                .nrs_dequeue = obrr_dequeue,
                .nrs_get_rcc = obrr_get_rcc,
                .nrs_finish  = obrr_finish,
                .nrs_stat_cleanup = obrr_stat_cleanup,
                .nrs_empty = nrs_empty,
                .nrs_init    = obrr_init,
                .nrs_exit    = obrr_exit,
                .nrs_priority = obrr_priority,
                .nrs_update_clnr = obrr_update_clnr,
        },

        .nt_name ="obrr",
};

/**
 * Congest Control.
 */

#define RCC_MIN 1
#define RCC_DEFAULT 8
#define RCC_MAX 32

struct congest_control {
        unsigned int    cc_diskbw;
        unsigned int    cc_iops;
        unsigned int    cc_max_iops;
        unsigned int    cc_completed_ios;
        unsigned int    cc_qmax;
        unsigned int    cc_qmin;
        unsigned int    cc_maxlat;
        unsigned int    cc_lat;
        unsigned int    cc_prevlat;
        unsigned int    cc_sample_cnt;
        unsigned int    cc_depth;
        obd_count       cc_iosize; /* pending I/O size */
        obd_count       cc_max_iosize;
        int             cc_max_frcc;
        int             cc_rccmax;
        int             cc_rccmin;
        int             cc_rccdef;
        int             cc_prevrcc;
        int             cc_return_rcc;
        int             cc_last_sample_time;
        int             cc_sample_intv;
        int             cc_last_updated_time;
        int             cc_updated_intv;
        int             cc_algo;
        int             cc_delta;
        obd_count       cc_comp_iosize;
        int             cc_arrival_cnt;
        int             cc_arrival_rate;
        int             cc_rate;

        long            cc_wait_time;
        long            cc_work_time;

        unsigned int    cc_active_intv;
        atomic_t        cc_active_clnr;
};

/**
 * FCFS NRS algorithm: (using for RPC trace only).
 */
struct fcfs_data {
        int                     ff_ccon;
        struct congest_control  ff_cc;

        spinlock_t              ff_lock;
        struct nrs              *ff_nrs;
        struct list_head        ff_queue;
        cfs_proc_dir_entry_t    *ff_proc_entry;
};

static int fcfs_enqueue(struct nrs *s, struct ptlrpc_request *req)
{
        struct fcfs_data *ffd = s->nrs_data;
        int opc;
        ENTRY;

        opc = lustre_msg_get_opc(req->rq_reqmsg);
        if (opc == OST_READ || opc == OST_WRITE) {
                nrs_parse_ioreq(req, NRS_ENQUEUE);
                if (1) { /* ccon */
                        struct obd_export *exp = req->rq_export;

                        spin_lock(&ffd->ff_lock);
                        ffd->ff_cc.cc_iosize += req->rq_nrs_node.pnn_end -
                                req->rq_nrs_node.pnn_start + 1;
                        ffd->ff_cc.cc_max_iosize = max(ffd->ff_cc.cc_max_iosize,
                                ffd->ff_cc.cc_iosize);
                        ffd->ff_cc.cc_arrival_cnt++;
                        /*CDEBUG(D_RPCTRACE, "Enqueue an I/O request xid "LPU64
                                " from %s objid %llu Extent ["
                                LPU64" -- "LPU64"], current queued number: %d\n",
                                req->rq_xid,
                                libcfs_id2str(req->rq_peer),
                                req->rq_nrs_node.pnn_obj->ioo_id,
                                req->rq_nrs_node.pnn_start,
                                req->rq_nrs_node.pnn_end,
                                s->nrs_queued_num + 1);*/
                        if (!exp->exp_rw_active&&atomic_read(&exp->exp_rw_in_flight) == 0) {
                                atomic_inc(&ffd->ff_cc.cc_active_clnr);
                                exp->exp_rw_active = 1;
                                CDEBUG(D_RPCTRACE, "add client %p", exp);
                        }
                        atomic_inc(&req->rq_export->exp_rw_in_flight);
                        spin_unlock(&ffd->ff_lock);
                } else
                        nrs_trace_ioreq(s, req, NRS_ENQUEUE);

        }
        s->nrs_queued_num++;
        s->nrs_max_depth = max(s->nrs_max_depth, s->nrs_queued_num);
        list_add_tail(&req->rq_list, &ffd->ff_queue);
        req->rq_nrs = 1;

        RETURN(REQ_NRS_ACCEPT);
}

static struct ptlrpc_request *fcfs_dequeue(struct nrs *s)
{
        struct fcfs_data *ffd = s->nrs_data;
        struct ptlrpc_request *req;
        int opc;
        ENTRY;

        if (list_empty(&ffd->ff_queue)) {
                RETURN(NULL);
        }

        req = list_entry(ffd->ff_queue.next, struct ptlrpc_request, rq_list);
        list_del_init(&req->rq_list);
        s->nrs_queued_num--;
        LASSERT(req->rq_nrs == 1);
        req->rq_nrs = 0;
        if (!ffd->ff_ccon) {
                opc = lustre_msg_get_opc(req->rq_reqmsg);
                if (opc == OST_READ || opc == OST_WRITE) {
                        nrs_trace_ioreq(s, req, NRS_DEQUEUE);
                }
        }

        RETURN(req);
}

static void fcfs_finish(struct nrs *s, struct ptlrpc_request *req)
{
        struct fcfs_data *ffd = (struct fcfs_data *) s->nrs_data;
        struct timeval now;
        long timediff;
        int opc;

        if (req->rq_phase != RQ_PHASE_COMPLETE)
                return;

        do_gettimeofday(&now);
        timediff = cfs_timeval_sub(&now, &req->rq_arrival_time, NULL);

        spin_lock(&ffd->ff_lock);
        s->nrs_max_serv_time = max(s->nrs_max_serv_time, timediff);
        opc = lustre_msg_get_opc(req->rq_reqmsg);
        if (opc == OST_READ || opc == OST_WRITE) {
                obd_count size;
                struct obd_export *exp = req->rq_export;

                ffd->ff_cc.cc_completed_ios++;
                ffd->ff_cc.cc_lat = timediff / ONE_MILLION;
                size = (req->rq_nrs_node.pnn_end -
                                     req->rq_nrs_node.pnn_start + 1);
                ffd->ff_cc.cc_iosize -= size;
                ffd->ff_cc.cc_comp_iosize += size;

                ffd->ff_cc.cc_wait_time = req->rq_wait_time;
                ffd->ff_cc.cc_work_time = req->rq_work_time;

                if (atomic_dec_and_test(&exp->exp_rw_in_flight)) {
                        exp->exp_last_active_time = now.tv_sec;
                }

                CDEBUG(D_RPCTRACE, "Finished an I/O request xid "LPU64
                        " from %s objid %llu Extent ["LPU64" -- "LPU64"], current queued number: %d"
                        " server time %u\n",
                        req->rq_xid,
                        libcfs_id2str(req->rq_peer),
                        req->rq_nrs_node.pnn_obj->ioo_id,
                        req->rq_nrs_node.pnn_start,
                        req->rq_nrs_node.pnn_end,
                        s->nrs_queued_num,
                        ffd->ff_cc.cc_lat);
        }
        spin_unlock(&ffd->ff_lock);
        return;
}

static void init_congestcontrol(struct congest_control *cc)
{
        memset(cc, 0, sizeof(*cc));
        cc->cc_qmin = 128;
        cc->cc_maxlat = 25;
        cc->cc_diskbw = 180;
        cc->cc_rccmax = RCC_MAX;
        cc->cc_rccmin = RCC_MIN;
        cc->cc_rccdef = RCC_DEFAULT;
        cc->cc_prevrcc = cc->cc_rccdef;
        cc->cc_last_sample_time = cc->cc_last_updated_time = cfs_time_current_sec();
        cc->cc_sample_intv = 60;
        cc->cc_updated_intv = 1;
        cc->cc_max_iops = 206;
        LASSERT(cc->cc_rccmin >= 1);

        atomic_set(&cc->cc_active_clnr, 0);
        cc->cc_active_intv = 60;
}

static void fcfs_stat_cleanup(struct nrs *s)
{
        struct fcfs_data *ffd = s->nrs_data;

        init_congestcontrol(&ffd->ff_cc);
        return;
}

static int lprocfs_fcfs_rd_ccon(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%d\n", ffd->ff_ccon);
        return rc;
}

static int lprocfs_fcfs_wr_ccon(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val != 0 && val != 1)
                return -EINVAL;

        ffd->ff_ccon = val;
        return count;
}

static int lprocfs_fcfs_rd_diskbw(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_diskbw);
        return rc;
}

static int lprocfs_fcfs_wr_diskbw(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if ((val <= 0 && val > 2000))
                return -EINVAL;

        ffd->ff_cc.cc_diskbw = val;
        return count;
}

static int lprocfs_fcfs_rd_qmax(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_qmax);
        return rc;
}

static int lprocfs_fcfs_wr_qmax(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val < 0 )
                return -EINVAL;

        ffd->ff_cc.cc_qmax = val;
        ffd->ff_cc.cc_depth = ffd->ff_cc.cc_qmax;
        return count;
}

static int lprocfs_fcfs_rd_qmin(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_qmin);
        return rc;
}

static int lprocfs_fcfs_wr_qmin(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if ((val < 0 ))
                return -EINVAL;

        ffd->ff_cc.cc_qmin = val;
        return count;
}

static int lprocfs_fcfs_rd_max_latency(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_maxlat);
        return rc;
}

static int lprocfs_fcfs_wr_max_latency(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if ((val < 0 ))
                return -EINVAL;

        ffd->ff_cc.cc_maxlat = val;
        return count;
}

static int lprocfs_fcfs_rd_max_iosize(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_max_iosize);
        return rc;
}

static int lprocfs_fcfs_wr_max_iosize(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val < 0)
                return -EINVAL;

        ffd->ff_cc.cc_max_iosize = val;
        return count;
}

static int lprocfs_fcfs_rd_max_frcc(char *page, char **start, off_t off,
                        int count, int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_max_frcc);
        return rc;
}

static int lprocfs_fcfs_wr_max_frcc(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if ((val <= 0 ))
                return -EINVAL;

        ffd->ff_cc.cc_max_frcc = val;
        return count;
}

static int lprocfs_fcfs_rd_rccmax(char *page, char **start, off_t off,
                        int count, int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_rccmax);
        return rc;
}

static int lprocfs_fcfs_wr_rccmax(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if ((val <= 0 ))
                return -EINVAL;

        ffd->ff_cc.cc_rccmax = val;
        return count;
}

static int lprocfs_fcfs_rd_rccmin(char *page, char **start, off_t off,
                        int count, int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_rccmin);
        return rc;
}

static int lprocfs_fcfs_wr_rccmin(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if ((val <= 0 ))
                return -EINVAL;

        ffd->ff_cc.cc_rccmin = val;
        return count;
}

static int lprocfs_fcfs_rd_rccdef(char *page, char **start, off_t off,
                        int count, int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_rccdef);
        return rc;
}

static int lprocfs_fcfs_wr_rccdef(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if ((val <= 0 ))
                return -EINVAL;

        ffd->ff_cc.cc_rccdef = val;
        return count;
}

static int lprocfs_fcfs_rd_info(char *page, char **start, off_t off,
                        int count, int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u %u %d %u %u %d %ld %ld %d\n",
                ffd->ff_nrs->nrs_queued_num, ffd->ff_cc.cc_lat,
                ffd->ff_cc.cc_return_rcc, ffd->ff_cc.cc_diskbw,
                ffd->ff_cc.cc_iops, ffd->ff_cc.cc_arrival_rate,
                ffd->ff_cc.cc_wait_time, ffd->ff_cc.cc_work_time,
                atomic_read(&ffd->ff_cc.cc_active_clnr));
        return rc;
}

static int lprocfs_fcfs_rd_algo(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_algo);
        return rc;
}

static int lprocfs_fcfs_wr_algo(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val <= CC_ALGO_MIN || val >= CC_ALGO_MAX )
                return -EINVAL;

        ffd->ff_cc.cc_algo = val;
        return count;
}

static int lprocfs_fcfs_rd_active_intv(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_active_intv);
        return rc;
}

static int lprocfs_fcfs_wr_active_intv(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val <= 0 )
                return -EINVAL;

        ffd->ff_cc.cc_active_intv = val;
        return count;
}

static int lprocfs_fcfs_rd_sample_intv(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_sample_intv);
        return rc;
}

static int lprocfs_fcfs_wr_sample_intv(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val <= 0)
                return -EINVAL;

        ffd->ff_cc.cc_sample_intv = val;
        return count;
}

static int lprocfs_fcfs_rd_updated_intv(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_updated_intv);
        return rc;
}

static int lprocfs_fcfs_wr_updated_intv(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val <= 0)
                return -EINVAL;

        ffd->ff_cc.cc_updated_intv = val;
        return count;
}

static int lprocfs_fcfs_rd_max_iops(char *page, char **start, off_t off, int count,
                        int *eof, void *data)
{
        struct fcfs_data *ffd = data;
        int rc;

        rc = snprintf(page, count, "%u\n", ffd->ff_cc.cc_max_iops);
        return rc;
}

static int lprocfs_fcfs_wr_max_iops(struct file *file, const char *buffer,
                unsigned long count, void *data)
{
        struct fcfs_data *ffd = data;
        int val;
        int rc;

        rc = lprocfs_write_helper(buffer, count, &val);
        if (rc)
                return rc;

        if (val <= 0)
                return -EINVAL;

        ffd->ff_cc.cc_max_iops = val;
        return count;
}

struct lprocfs_vars lprocfs_fcfs_vars[] = {
        { "cc_on", lprocfs_fcfs_rd_ccon, lprocfs_fcfs_wr_ccon, 0 },
        { "disk_bandwidth", lprocfs_fcfs_rd_diskbw, lprocfs_fcfs_wr_diskbw, 0 },
        { "qmax", lprocfs_fcfs_rd_qmax, lprocfs_fcfs_wr_qmax, 0 },
        { "qmin", lprocfs_fcfs_rd_qmin, lprocfs_fcfs_wr_qmin, 0 },
        { "max_latency", lprocfs_fcfs_rd_max_latency, lprocfs_fcfs_wr_max_latency, 0 },
        { "max_iosize", lprocfs_fcfs_rd_max_iosize, lprocfs_fcfs_wr_max_iosize, 0 },
        { "max_feedback_rcc", lprocfs_fcfs_rd_max_frcc, lprocfs_fcfs_wr_max_frcc, 0 },
        { "max_rcc", lprocfs_fcfs_rd_rccmax, lprocfs_fcfs_wr_rccmax, 0 },
        { "min_rcc", lprocfs_fcfs_rd_rccmin, lprocfs_fcfs_wr_rccmin, 0 },
        { "def_rcc", lprocfs_fcfs_rd_rccdef, lprocfs_fcfs_wr_rccdef, 0 },
        { "info", lprocfs_fcfs_rd_info, 0, 0 },
        { "algo", lprocfs_fcfs_rd_algo, lprocfs_fcfs_wr_algo, 0 },
        { "interval", lprocfs_fcfs_rd_sample_intv, lprocfs_fcfs_wr_sample_intv, 0 },
        { "update_interval", lprocfs_fcfs_rd_updated_intv, lprocfs_fcfs_wr_updated_intv, 0 },
        { "max_iops", lprocfs_fcfs_rd_max_iops, lprocfs_fcfs_wr_max_iops, 0},
        { "active_interval", lprocfs_fcfs_rd_active_intv, lprocfs_fcfs_wr_active_intv, 0},
        { 0 }
};

static int fcfs_procfs_init(struct nrs *s, struct fcfs_data *ffd)
{
        int rc = 0;

        ffd->ff_proc_entry = lprocfs_register("fcfs", s->nrs_proc_entry,
                                lprocfs_fcfs_vars, ffd);
        if (IS_ERR(ffd->ff_proc_entry)) {
                rc = PTR_ERR(ffd->ff_proc_entry);
                CERROR("Error %d setting up lprocfs for obrr\n", rc);
                ffd->ff_proc_entry = NULL;
                return rc;
        }
        return rc;
}

static void fcfs_procfs_fini(struct fcfs_data *ffd)
{
        if (ffd->ff_proc_entry) {
                lprocfs_remove(&ffd->ff_proc_entry);
                ffd->ff_proc_entry = NULL;
        }
}

static void fcfs_exit(struct nrs *s)
{
        struct fcfs_data *ffd = s->nrs_data;

        if (ffd != NULL) {
                LASSERT(list_empty(&ffd->ff_queue));
                fcfs_procfs_fini(ffd);
                OBD_FREE_PTR(ffd);
        }
}

static int fcfs_init(struct nrs *s)
{
        struct fcfs_data *ffd;
        int rc;

        OBD_ALLOC_PTR(ffd);
        if (ffd == NULL)
                GOTO(out, rc = -ENOMEM);

        CFS_INIT_LIST_HEAD(&ffd->ff_queue);
        ffd->ff_ccon = 1;
        init_congestcontrol(&ffd->ff_cc);
        LASSERT(ffd->ff_cc.cc_rccmin >= 1);

        spin_lock_init(&ffd->ff_lock);
        ffd->ff_nrs = s;
        rc = fcfs_procfs_init(s, ffd);
out:
        if (rc)
                fcfs_exit(s);

        s->nrs_data = ffd;

        return rc;
}

static void fcfs_priority(struct nrs *s, struct ptlrpc_request *req,
                          struct list_head *priority_list)
{
        if (req->rq_nrs == 1) {
                int opc;
                s->nrs_queued_num--;
                list_del_init(&req->rq_list);
                opc = lustre_msg_get_opc(req->rq_reqmsg);
                if (opc == OST_READ || opc == OST_WRITE)
                        nrs_trace_ioreq(s, req, NRS_DEQUEUE);

        }
        list_move_tail(&req->rq_list, priority_list);
        req->rq_hp = 1;
}

static int fcfs_get_rcc(struct nrs *nrs, int nr)
{
        struct fcfs_data *ffd = nrs->nrs_data;
        struct congest_control *cc = &ffd->ff_cc;
        int rcc = 0;
        time_t now = cfs_time_current_sec();

        if (now - cc->cc_last_sample_time > cc->cc_sample_intv) {
                spin_lock(&ffd->ff_lock);
                cc->cc_last_sample_time = now;
                cc->cc_iops = min(cc->cc_completed_ios / cc->cc_sample_intv, cc->cc_max_iops);
                cc->cc_completed_ios = 0;

                cc->cc_diskbw = cc->cc_comp_iosize / cc->cc_sample_intv;
                cc->cc_comp_iosize = 0;

                cc->cc_arrival_rate == cc->cc_arrival_cnt / cc->cc_sample_intv;
                cc->cc_arrival_cnt = 0;
                spin_unlock(&ffd->ff_lock);
        }

        if (ffd->ff_ccon == 0) {
                goto out;
        }

        if (nrs->nrs_queued_num < cc->cc_qmin)
                rcc = min((int)(cc->cc_qmin - nrs->nrs_queued_num), nr);
        else if (cc->cc_lat == cc->cc_maxlat) {
                        rcc = cc->cc_prevrcc;
                        goto out;
        } else if (now - cc->cc_last_updated_time > cc->cc_updated_intv){
                /* if updated interval is zero, update the returuned RCC for each I/O request */
                int base_rcc;
                unsigned int qdepth;
                int clnr = atomic_read(&cc->cc_active_clnr);
                unsigned int qlat, elat;

                LASSERT(clnr >= 1);

                cc->cc_last_updated_time = now;
                qdepth = cc->cc_iops * cc->cc_maxlat;
                base_rcc = qdepth / clnr;

                qlat = nrs->nrs_queued_num / cc->cc_iops;
                elat = max(qlat, cc->cc_lat);
                //rcc = base_rcc * cc->cc_maxlat / elat;
                if (elat > cc->cc_maxlat)
                        rcc = base_rcc - 1;
                else if (elat < cc->cc_maxlat) {
                        rcc = base_rcc;
                } else
                        rcc = cc->cc_prevrcc;
        }

        rcc = min(max(rcc, cc->cc_rccmin), cc->cc_rccmax);
        cc->cc_max_frcc = max(cc->cc_max_frcc, rcc);
        cc->cc_prevrcc = rcc;
out:
        cc->cc_return_rcc = rcc;
        CDEBUG(D_RPCTRACE, "Return RCC %d, expect depth %u, real depth %u\n",
                rcc, cc->cc_depth, nrs->nrs_queued_num);
        return rcc;
}

int static fcfs_update_clnr(struct nrs *s, struct ptlrpc_request *req)
{
        struct fcfs_data *ffd = s->nrs_data;
        struct obd_export *exp = req->rq_export;

        spin_lock(&ffd->ff_lock);
        if (exp->exp_rw_active && atomic_read(&exp->exp_rw_in_flight) == 0) {
                time_t now = cfs_time_current_sec();

                if (now - req->rq_export->exp_last_active_time >= ffd->ff_cc.cc_active_intv) {
                        atomic_dec(&ffd->ff_cc.cc_active_clnr);
                        exp->exp_rw_active = 0;
                        CDEBUG(D_RPCTRACE, "delete client %p", exp);
                }
        }
        spin_unlock(&ffd->ff_lock);

        return 0;
}

static struct nrs_type nrs_fcfs = {
        .nt_ops = {
                .nrs_enqueue = fcfs_enqueue,
                .nrs_dequeue = fcfs_dequeue,
                .nrs_get_rcc = fcfs_get_rcc,
                .nrs_finish  = fcfs_finish,
                .nrs_stat_cleanup = fcfs_stat_cleanup,
                .nrs_empty = nrs_empty,
                .nrs_init    = fcfs_init,
                .nrs_exit    = fcfs_exit,
                .nrs_priority = fcfs_priority,
                .nrs_update_clnr = fcfs_update_clnr,
        },
        .nt_name ="fcfs",
};

void ost_register_nrs_types(void)
{
        nrs_register(&nrs_obrr);
        nrs_register(&nrs_fcfs);
}

int ost_io_nrs_init(struct ptlrpc_service *svc)
{
        return nrs_attach(svc, "fcfs");
}

