#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
# description: 指导Lustre client和server端进行调度的rule
#      author: ShijunDeng
#       email: dengshijun1992@gmail.com
#        time: 2017-10-07
#
#
# 根据qos_rule参数指导调度
#


class qos_rule:
    def __init__(self, ack_ewma_lower=0, ack_ewma_upper=0, send_ewma_lower=0, send_ewma_upper=0, rtt_ratio_lower=0,
                 rtt_ratio_upper=0, m=0, b=0, tau=0, used_times=0, ack_ewma_avg=0, send_ewma_avg=0, rtt_ratio_avg=0):
        self.ack_ewma_lower = ack_ewma_lower
        self.ack_ewma_upper = ack_ewma_upper
        self.send_ewma_lower = send_ewma_lower
        self.send_ewma_upper = send_ewma_upper
        # self.invl_ewma_lower = invl_ewma_lower
        # self.invl_ewma_upper = invl_ewma_upper
        self.rtt_ratio_lower = rtt_ratio_lower
        self.rtt_ratio_upper = rtt_ratio_upper
        # < m, b, τ >
        self.m = m
        self.b = b
        self.tau = tau
        self.used_times = used_times
        self.ack_ewma_avg = ack_ewma_avg
        self.send_ewma_avg = send_ewma_avg
        self.rtt_ratio100_avg = rtt_ratio_avg

    #
    # 输出qos_rule的参数名以及对应的参数值
    #
    def printf_kv(self):
        print("ack_ewma (%d, %d)" % (self.ack_ewma_lower, self.ack_ewma_upper))
        print("send_ewma (%d, %d)" % (self.send_ewma_lower, self.send_ewma_upper))
        # print("invl_ewma_lower = %d,invl_ewma_upper = %d" % (self.invl_ewma_lower, self.invl_ewma_upper))
        print("rtt_ratio (%d, %d)" % (self.rtt_ratio_lower, self.rtt_ratio_upper))
        print("<m,b,tau> = < %d, %d, %d >" % (self.m, self.b, self.tau))
        print("used_times = %d" % self.used_times)
        print("ack_ewma_avg = %d,send_ewma_avg = %d,rtt_ratio_avg = %d" % (self.ack_ewma_avg, self.send_ewma_avg,
                                                                           self.rtt_ratio_avg))
