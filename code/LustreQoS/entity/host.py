#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
# description: ip地址，域名，主机名之间的处理
#      author: ShijunDeng
#       email: dengshijun1992@gmail.com
#        time: 2017-10-10
#


#
# 标志一个文件系统节点
#
class hostnode:
    def __init__(self):
        self.ip = None
        self.domain = None
        self.hostname = None
        self.property = None  # oss mds client等

    def set_values(self, ip, domain, hostname, property):
        self.ip = ip
        self.domain = domain
        self.hostname = hostname
        self.property = property

    def printf_kv(self):
        print("ip:%s, domain:%s, hostname:%s, property:%s" % (self.ip, self.domain, self.hostname, self.property))


class host_set:
    def __init__(self):
        self.hosts = {}

    #
    # 查找ip地址所指的hostnode是否存在
    #
    def search(self, ip):
        if self.hosts[ip] is not None:
            return self.hosts[ip]
        return None

    def add(self, e):
        self.hosts[e.ip] = e

    def add_hostlist(self, e_list):
        if (isinstance(e_list, list)):
            for e in e_list:
                self.add(e)

    def delete_by_ip(self, ip):
        del self.hosts[ip]

    def printf_kv(self):
        for host in self.hosts.values():
            host.printf_kv()


def is_oss_node(hostnode):
    return hostnode.property == "oss"


def is_client_node(hostnode):
    return hostnode.property == "client"


def is_mds_node(hostnode):
    return hostnode.property == "mds"
