#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
# description: ltop -r命令结果中的mdt和ost数据
#      author: ShijunDeng
#       email: dengshijun1992@gmail.com
#        time: 2017-10-11
#
#

class ltop_base:
    def __init__(self):
        self.tnow = 0
        self.trcv = 0
        self.node = ""
        self.name = ""
        self.uts_nodename = ""
        self.cpupct = 0
        self.mempct = 0
        self.uuid = ""
        self.filesfree = 0
        self.filestotal = 0
        self.kbytesfree = 0
        self.kbytestotal = 0

    def printf_kv(self):
        print("tnow:%d" % self.tnow)
        print("trcv:%d" % self.trcv)
        print("node:%s" % self.node)
        print("name:%s" % self.name)
        print("uts_nodename:%s" % self.uts_nodename)
        print("cpupct:%f%%" % self.cpupct)
        print("mempct:%f%%" % self.mempct)
        print("uuid:%s" % self.uuid)
        print("filesfree:%d" % self.filesfree)
        print("filestotal:%d" % self.filestotal)
        print("kbytesfree:%d" % self.kbytesfree)
        print("kbytestotal:%d" % self.kbytestotal)


class ltop_mdt(ltop_base):
    def __init__(self):
        #
        # 每个属性有三个值：count, sum,sumsq
        #
        super().__init__()
        self.open = [-1, -1, -1]
        self.close = [-1, -1, -1]
        self.mknod = [-1, -1, -1]
        self.link = [-1, -1, -1]
        self.unlink = [-1, -1, -1]
        self.mkdir = [-1, -1, -1]
        self.rmdir = [-1, -1, -1]
        self.rename = [-1, -1, -1]
        self.getxattr = [-1, -1, -1]
        self.process_config = [-1, -1, -1]
        self.connect = [-1, -1, -1]
        self.reconnect = [-1, -1, -1]
        self.disconnect = [-1, -1, -1]
        self.statfs = [-1, -1, -1]
        self.create = [-1, -1, -1]
        self.destroy = [-1, -1, -1]
        self.setattr = [-1, -1, -1]
        self.getattr = [-1, -1, -1]
        self.llog_init = [-1, -1, -1]
        self.notify = [-1, -1, -1]
        self.quotactl = [-1, -1, -1]

    def printf_kv(self):
        super().printf_kv()
        print("open:%d,%d,%d" % (self.open[0], self.open[1], self.open[2]))
        print("close:%d,%d,%d" % (self.close[0], self.close[1], self.close[2]))
        print("mknod:%d,%d,%d" % (self.mknod[0], self.mknod[1], self.mknod[2]))
        print("link:%d,%d,%d" % (self.link[0], self.link[1], self.link[2]))
        print("unlink:%d,%d,%d" % (self.unlink[0], self.unlink[1], self.unlink[2]))
        print("mkdir:%d,%d,%d" % (self.mkdir[0], self.mkdir[1], self.mkdir[2]))
        print("rmdir:%d,%d,%d" % (self.rmdir[0], self.rmdir[1], self.rmdir[2]))
        print("rename:%d,%d,%d" % (self.rename[0], self.rename[1], self.rename[2]))
        print("getxattr:%d,%d,%d" % (self.getxattr[0], self.getxattr[1], self.getxattr[2]))
        print("process_config:%d,%d,%d" % (self.process_config[0], self.process_config[1], self.process_config[2]))
        print("connect:%d,%d,%d" % (self.connect[0], self.connect[1], self.connect[2]))
        print("reconnect:%d,%d,%d" % (self.reconnect[0], self.reconnect[1], self.reconnect[2]))
        print("disconnect:%d,%d,%d" % (self.disconnect[0], self.disconnect[1], self.disconnect[2]))
        print("statfs:%d,%d,%d" % (self.statfs[0], self.statfs[1], self.statfs[2]))
        print("create:%d,%d,%d" % (self.create[0], self.create[1], self.create[2]))
        print("destroy:%d,%d,%d" % (self.destroy[0], self.destroy[1], self.destroy[2]))
        print("setattr:%d,%d,%d" % (self.setattr[0], self.setattr[1], self.setattr[2]))
        print("getattr:%d,%d,%d" % (self.getattr[0], self.getattr[1], self.getattr[2]))
        print("llog_init:%d,%d,%d" % (self.llog_init[0], self.llog_init[1], self.llog_init[2]))
        print("notify:%d,%d,%d" % (self.notify[0], self.notify[1], self.notify[2]))
        print("quotactl:%d,%d,%d" % (self.quotactl[0], self.quotactl[1], self.quotactl[2]))


class ltop_ost(ltop_base):
    def __init__(self):
        super().__init__()
        self.read_bytes = 0
        self.write_bytes = 0
        self.iops = 0
        self.num_exports = 0
        self.lock_count = 0
        self.grant_rate = 0
        self.cancel_rate = 0
        self.connect = 0
        self.reconnect = 0
        self.recov_str = ""

    def printf_kv(self):
        super().printf_kv()
        print("read_bytes:%d" % self.read_bytes)
        print("write_bytes:%d" % self.write_bytes)
        print("iops:%d" % self.iops)
        print("num_exports:%d" % self.num_exports)
        print("lock_count:%d" % self.lock_count)
        print("grant_rate:%d" % self.grant_rate)
        print("cancel_rate:%d" % self.cancel_rate)
        print("connect:%d" % self.connect)
        print("reconnect:%d" % self.reconnect)
        print("recov_str:%s" % self.recov_str)
