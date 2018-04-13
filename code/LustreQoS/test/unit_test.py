#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
# description: 测试
#      author: ShijunDeng
#       email: dengshijun1992@gmail.com
#        time: 2017-10-22
#

from utils import *
from config import conf_tool
from utils import get_ltop_list,get_oss_node_list
from entity import hostnode, host_set
from metric import ost_hostnode_ltop
import os


#
# 产生etc_host.xml文件
#
def generate_etc_hostsxml():
    hosts_set_var = host_set()
    i = 0
    for i in range(7):
        host_var = hostnode()
        host_var.ip = "192.168.122.1" + str(i + 1)
        host_var.domain = host_var.hostname = "qosnode" + str(i + 1)
        if (i < 4):
            host_var.property = "oss"
        elif (i > 4):
            host_var.property = "client"
        else:
            host_var.property = "mds"
        hosts_set_var.add(host_var)
        write_etc_hosts(hosts_set_var)


#
#计算各节点带宽
#
def cal_detail():
    mds_list = get_oss_node_list()
    ost_hostnode_ltop_list = []
    ltop_data_list = get_ltop_list()
    ltop_ost_list = ltop_data_list['ostlist']
    ltop_mdt_list = ltop_data_list['mdtlist']
    for element in mds_list:
        ost_hostnode_ltop_var = ost_hostnode_ltop()
        ost_hostnode_ltop_var.hostnode_var = element

        ost_hostnode_ltop_var.filter_add_ltop_ost_list(ltop_ost_list)
        sorted(ost_hostnode_ltop_var.ltop_ost_list, key=lambda top_ost_var: top_ost_var.trcv, reverse=False)
        # ost_hostnode_ltop_var.sort_ltop_ost_list(lambda top_ost_var: top_ost_var.trcv)
        ost_hostnode_ltop_list.append(ost_hostnode_ltop_var)
        print("---------------%s---------------------" % element.hostname)
        i = 1
        while i < len(ost_hostnode_ltop_var.ltop_ost_list):
            br = (ost_hostnode_ltop_var.ltop_ost_list[i].read_bytes - ost_hostnode_ltop_var.ltop_ost_list[
                i - 1].read_bytes) / (
                     ost_hostnode_ltop_var.ltop_ost_list[i].trcv - ost_hostnode_ltop_var.ltop_ost_list[i - 1].trcv) / (
                     1024 * 1024)
            bw = (ost_hostnode_ltop_var.ltop_ost_list[i].write_bytes - ost_hostnode_ltop_var.ltop_ost_list[
                i - 1].write_bytes) / (
                     ost_hostnode_ltop_var.ltop_ost_list[i].trcv - ost_hostnode_ltop_var.ltop_ost_list[i - 1].trcv) / (
                     1024 * 1024)
            print("br=%f,bw=%f,filesfree=%d -> %d,filestotal=%d -> %d,sfree=%f -> %f,stotal=%f -> %f" % (
            br, bw, ost_hostnode_ltop_var.ltop_ost_list[i - 1].filesfree,
            ost_hostnode_ltop_var.ltop_ost_list[i].filesfree
            , ost_hostnode_ltop_var.ltop_ost_list[i - 1].filestotal, ost_hostnode_ltop_var.ltop_ost_list[i].filestotal,
            ost_hostnode_ltop_var.ltop_ost_list[
                i - 1].kbytesfree / (1024 * 1024), ost_hostnode_ltop_var.ltop_ost_list[
                i].kbytesfree / (1024 * 1024), ost_hostnode_ltop_var.ltop_ost_list[
                i - 1].kbytestotal / (1024 * 1024), ost_hostnode_ltop_var.ltop_ost_list[
                i].kbytestotal / (1024 * 1024)))
            i += 1
