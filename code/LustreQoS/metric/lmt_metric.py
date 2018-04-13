#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
# description: 根据lmt工具中监测的数据计算一些有用的指标例如bandwidth等等
#      author: ShijunDeng
#       email: dengshijun1992@gmail.com
#        time: 2017-10-14
# *
from entity import hostnode
from entity import ltop_mdt, ltop_ost
from utils import get_oss_node_list, get_ltop_list


#
# ost的node节点和对应的lmt中ltop命令的信息
#
class ost_hostnode_ltop:
    def __init__(self):
        self.hostnode_var = None
        self.ltop_ost_list = []

    #
    # 判断该条信息是否属于该hostnode
    #
    def __accept__(self, ltop_ost_var):
        return ltop_ost_var.node == self.hostnode_var.hostname

    #
    # 从ltop_ost_list_other中筛选出属于当前节点的ltop信息并添加到当前的ltop_ost_list中
    #
    def filter_add_ltop_ost_list(self, ltop_ost_list_other):
        for ltop_ost_var in ltop_ost_list_other:
            if self.__accept__(ltop_ost_var):
                self.ltop_ost_list.append(ltop_ost_var)

    #
    # 直接添加到当前hostnode的ltop_ost_list,不做判断
    #
    def add_ltop_host(self, ltop_ost_var):
        self.ltop_ost_list.append(ltop_ost_var)

    #
    # 添加的时候同时判断是否属于当前的hostnode,属于当前的hostnode才添加
    #
    def filter_add_ltop_ost(self, ltop_ost_var):
        if not ltop_ost_var and self.__accept__(ltop_ost_var):
            self.add_ltop_host(ltop_ost_var)

    def sort_ltop_ost_list(self, sort_key):
        sorted(self.ltop_ost_list, sort_key, reverse=False)


