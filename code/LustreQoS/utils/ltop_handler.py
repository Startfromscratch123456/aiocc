#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
# description: 解析ltop -r命令结果中的mdt和ost数据
#      author: ShijunDeng
#       email: dengshijun1992@gmail.com
#        time: 2017-10-13
#
#
from entity import ltop_mdt, ltop_ost
from config import conf_tool

import os


class ltop_handler:
    def __init__(self):
        self.ltop_mdt_list = []
        self.ltop_ost_list = []
        # 读取总行数
        self.total_lines = 0
        # 无效行数，取决于调用函数的参照对象
        # 调用parse：__is_mdt_linestr __is_ost_linestr均返回false的
        # 调用parse_ost：__is_ost_linestr返回false的
        # 调用parse_mdt：__is_mdt_linestr返回false的
        #
        self.invalid_lines = 0
        # 重复行数，取决于调用函数的参照对象
        # 调用parse：与已有的 ltop_ost_lis t或者 ltop_mdt_list 列表中元素重复的
        # 调用parse_ost：与已有的 ltop_ost_list 列表中元素重复的
        # 调用parse_mdt：与已有的 ltop_mdt_list 列表中元素重复的
        self.repeated_lines = 0

    #
    # 判断该行数据是否是ltop关于ost的数据
    #
    def __is_ost_linestr(self, line_str):
        if ("ost" in line_str or "OST" in line_str):
            return True
        return False

    def __is_mdt_linestr(self, line_str):
        if ("mdt" in line_str or "MDT" in line_str):
            return True
        return False

    #
    # 1476168063 1476168061 ca26 lmt_mdt 1;ca26;0.058324;6.050639;lustrefs-MDT0000;250019580;250019840;370667956;370751348;117;0;0;103;0;0;0;0;0;0;0;0;42;0;0;3;0;0;3;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;62;0;0;0;0;0;0;0;0;55;0;0;232;0;0;0;0;0;0;0;0;0;0;0
    # 转换一行数据为一个ltop_mdt对象
    #
    def __parse_mdt(self, line_str):
        ltop_mdt_var = ltop_mdt()
        #
        # 处理截止 1476168063 1476168061 ca26 lmt_mdt 1;之前的数据
        #
        sub_str = line_str[0:line_str.index(';')]
        sub_str_array = sub_str.split(' ')
        ltop_mdt_var.tnow = int(sub_str_array[0])
        ltop_mdt_var.trcv = int(sub_str_array[1])
        ltop_mdt_var.node = str(sub_str_array[2])
        ltop_mdt_var.name = sub_str_array[3] + ' ' + sub_str_array[4]

        #
        # 处理截止 1476168063 1476168061 ca26 lmt_mdt 1;之后的数据
        # ca26;0.058324;6.050639;lustrefs-MDT0000;250019580;250019840;370667956;370751348;117;0;0;103;......
        #
        sub_str = line_str[line_str.index(';') + 1:]
        sub_str_array = sub_str.split(';')
        index = 0
        ltop_mdt_var.uts_nodename = str(sub_str_array[index])
        index += 1
        ltop_mdt_var.cpupct = float(sub_str_array[index])
        index += 1
        ltop_mdt_var.mempct = float(sub_str_array[index])
        index += 1
        ltop_mdt_var.uuid = str(sub_str_array[index])
        index += 1
        ltop_mdt_var.filesfree = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.filestotal = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.kbytesfree = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.kbytestotal = int(sub_str_array[index])

        #
        # attribute(count,sum,sumsq)
        #
        index += 1
        ltop_mdt_var.open[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.open[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.open[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.close[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.close[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.close[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.mknod[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.mknod[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.mknod[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.link[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.link[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.link[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.unlink[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.unlink[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.unlink[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.mkdir[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.mkdir[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.mkdir[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.rmdir[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.rmdir[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.rmdir[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.rename[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.rename[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.rename[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.getxattr[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.getxattr[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.getxattr[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.process_config[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.process_config[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.process_config[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.connect[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.connect[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.connect[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.reconnect[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.reconnect[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.reconnect[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.disconnect[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.disconnect[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.disconnect[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.statfs[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.statfs[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.statfs[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.create[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.create[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.create[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.destroy[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.destroy[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.destroy[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.setattr[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.setattr[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.setattr[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.getattr[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.getattr[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.getattr[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.llog_init[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.llog_init[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.llog_init[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.notify[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.notify[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.notify[2] = int(sub_str_array[index])

        index += 1
        ltop_mdt_var.quotactl[0] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.quotactl[1] = int(sub_str_array[index])
        index += 1
        ltop_mdt_var.quotactl[2] = int(sub_str_array[index])
        # ltop_mdt_var.printf_kv()
        return ltop_mdt_var

    #
    # 1476168059 1476168058 ca04 lmt_ost 2;
    # ca04;1.021690;47.522704;lustrefs-OST0001;7324418;7324800;483694268;497747696;185945354240;465287233536;666724;4;102;0;0;2;31;COMPLETE 1/3 0s remaining;
    # 转换一行数据为一个ltop_ost对象
    #
    def __parse_ost(self, line_str):
        ltop_ost_var = ltop_ost()
        #
        # 处理截止 1476168059 1476168058 ca04 lmt_ost 2;之前的数据
        #
        sub_str = line_str[0:line_str.index(';')]
        sub_str_array = sub_str.split(' ')
        ltop_ost_var.tnow = int(sub_str_array[0])
        ltop_ost_var.trcv = int(sub_str_array[1])
        ltop_ost_var.node = str(sub_str_array[2])
        ltop_ost_var.name = sub_str_array[3] + ' ' + sub_str_array[4]

        #
        # 处理截止 1476168059 1476168058 ca04 lmt_ost 2;之后的数据
        # ca04;1.021690;47.522704;lustrefs-OST0001;7324418;7324800;483694268;497747696;185945354240;465287233536;666724;4;102;0;0;2;31;COMPLETE 1/3 0s remaining;
        #
        sub_str = line_str[line_str.index(';') + 1:]
        sub_str_array = sub_str.split(';')
        index = 0
        ltop_ost_var.uts_nodename = str(sub_str_array[index])
        index += 1
        ltop_ost_var.cpupct = float(sub_str_array[index])
        index += 1
        ltop_ost_var.mempct = float(sub_str_array[index])
        index += 1
        ltop_ost_var.uuid = str(sub_str_array[index])
        index += 1
        ltop_ost_var.filesfree = int(sub_str_array[index])
        index += 1
        ltop_ost_var.filestotal = int(sub_str_array[index])
        index += 1
        ltop_ost_var.kbytesfree = int(sub_str_array[index])
        index += 1
        ltop_ost_var.kbytestotal = int(sub_str_array[index])
        index += 1
        ltop_ost_var.read_bytes = int(sub_str_array[index])
        index += 1
        ltop_ost_var.write_bytes = int(sub_str_array[index])
        index += 1
        ltop_ost_var.iops = int(sub_str_array[index])
        index += 1
        ltop_ost_var.num_exports = int(sub_str_array[index])
        index += 1
        ltop_ost_var.lock_count = int(sub_str_array[index])
        index += 1
        ltop_ost_var.grant_rate = int(sub_str_array[index])
        index += 1
        ltop_ost_var.cancel_rate = int(sub_str_array[index])
        index += 1
        ltop_ost_var.connect = int(sub_str_array[index])
        index += 1
        ltop_ost_var.reconnect = int(sub_str_array[index])
        index += 1
        ltop_ost_var.recov_str = str(sub_str_array[index])

        return ltop_ost_var

    # 以trcv作为key值判断元素是否有重复
    def __judge_distinct_by_trcv(self, list_var, other):
        for element in list_var:
            if element.trcv == other.trcv and element.node == other.node and element.name == other.name and element.uts_nodename == other.uts_nodename and element.uuid == other.uuid:
                self.repeated_lines += 1
                # print("检测到重复数据行......")
                # other.printf_kv()
                return True
        return False
        # map(lambda ltop_ost_var: ltop_ost_var.trcv == other.trcv, list_var)

    def parse_mdt(self, ltop_file_path):
        with open(ltop_file_path, "r") as ltop_file:
            line_str = ltop_file.readline()
            self.total_lines += 1
            while line_str:
                while line_str and not self.__is_mdt_linestr(line_str):
                    line_str = ltop_file.readline()
                    self.total_lines += 1
                    self.invalid_lines += 1
                if self.__is_mdt_linestr(line_str):
                    ltop_mdt_var = self.__parse_mdt(line_str)
                    if not self.__judge_distinct_by_trcv(self.ltop_mdt_list, ltop_mdt_var):
                        self.ltop_mdt_list.append(ltop_mdt_var)
                line_str = ltop_file.readline()
                self.total_lines += 1

    def parse_ost(self, ltop_file_path):
        with open(ltop_file_path, "r") as ltop_file:
            line_str = ltop_file.readline()
            self.total_lines += 1
            while line_str:
                while line_str and not self.__is_ost_linestr(line_str):
                    line_str = ltop_file.readline()
                    self.total_lines += 1
                    self.invalid_lines += 1
                if self.__is_ost_linestr(line_str):
                    ltop_ost_var = self.__parse_ost(line_str)
                    if not self.__judge_distinct_by_trcv(self.ltop_ost_list, ltop_ost_var):
                        self.ltop_ost_list.append(ltop_ost_var)
                line_str = ltop_file.readline()
                self.total_lines += 1

    def parse(self, ltop_file_path):
        with open(ltop_file_path, "r") as ltop_file:
            line_str = ltop_file.readline()
            self.total_lines += 1
            while line_str:
                # 跳出空行
                while not line_str:
                    line_str = ltop_file.readline()
                    self.total_lines += 1
                if self.__is_mdt_linestr(line_str):
                    ltop_mdt_var = self.__parse_mdt(line_str)
                    if not self.__judge_distinct_by_trcv(self.ltop_mdt_list, ltop_mdt_var):
                        self.ltop_mdt_list.append(ltop_mdt_var)
                elif self.__is_ost_linestr(line_str):
                    ltop_ost_var = self.__parse_ost(line_str)
                    if not self.__judge_distinct_by_trcv(self.ltop_ost_list, ltop_ost_var):
                        self.ltop_ost_list.append(ltop_ost_var)
                else:
                    print("Error:Unkown type,line_str:%s" % line_str)
                    self.invalid_lines += 1
                    # exit(-1)

                line_str = ltop_file.readline()
                self.total_lines += 1


#
# 过滤ltop_file中的无效行
#
def ltop_file_filter():
    ltop_file_path = conf_tool.get_base_path() + os.path.sep + conf_tool.get_ltop_file_path()
    filtered_content = ''
    with open(ltop_file_path, "r+") as ltop_file:
        line_str = ltop_file.readline()
        while line_str:
            if line_str and ("OST" in line_str or "ost" in line_str or "MDT" in line_str or "mdt" in line_str):
                filtered_content += line_str
            line_str = ltop_file.readline()
    with open(ltop_file_path, "w") as ltop_file:
        ltop_file.write(filtered_content)


#
# 只获取ltop命令结果中ost相关的信息，以list形式返回
#
def get_ltop_ostlist():
    ltop_handler_var = ltop_handler()
    ltop_handler_var.parse_ost(conf_tool.get_base_path() + os.path.sep + conf_tool.get_ltop_file_path())
    return ltop_handler_var.ltop_ost_list


#
# 只获取ltop命令结果中mdt相关的信息，以list形式返回
#
def get_ltop_mdtlist():
    ltop_handler_var = ltop_handler()
    ltop_handler_var.parse_mdt(conf_tool.get_base_path() + os.path.sep + conf_tool.get_ltop_file_path())
    return ltop_handler_var.ltop_mdt_list


#
# 获取ltop命令结果中mdt && ost相关的信息，以dict形式返回
#
def get_ltop_list():
    ltop_data_list = {}
    ltop_handler_var = ltop_handler()
    ltop_handler_var.parse(conf_tool.get_base_path() + os.path.sep + conf_tool.get_ltop_file_path())
    ltop_data_list['ostlist'] = ltop_handler_var.ltop_ost_list
    ltop_data_list['mdtlist'] = ltop_handler_var.ltop_mdt_list
    print("total_lines:%d, invalid_lines:%d, repeated_lines:%d" % (
        ltop_handler_var.total_lines, ltop_handler_var.invalid_lines, ltop_handler_var.repeated_lines))
    return ltop_data_list
