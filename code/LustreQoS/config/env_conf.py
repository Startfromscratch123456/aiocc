#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
# description: 提供工程配置的相关接口
#      author: ShijunDeng
#       email: dengshijun1992@gmail.com
#        time: 2016-10-07
#

import configparser
import os
import codecs

database_conf = "config" + os.path.sep + "database.conf"


class conf_ini:
    """
    注意：Key的值只能是小写
    """

    def __init__(self, filename, strcode=""):
        self.config = configparser.ConfigParser()
        self.fileName = filename
        if not strcode:
            self.config.read(self.fileName)
        else:
            if strcode.lower() == "utf-8-sig" or strcode.lower() == "utf-8" or strcode.lower() == "utf-16":
                with codecs.open(filename, "r", strcode) as fp:
                    if not fp:
                        raise "config file maybe not exist."
                    else:
                        self.config.read_file(fp)
            else:
                self.config.read(self.fileName)

    def write_ini(self, section_name, key_name, value):
        if section_name not in self.config.sections():
            self.config.add_section(section_name)
        self.config.set(section_name, key_name, value)
        with open(self.fileName, "w") as file_var:
            self.config.write(file_var)

    def read_ini_str(self, section_name, key_name):
        key_name = key_name.lower()
        if section_name in self.config.sections() and key_name in self.config.options(section_name):
            return self.config.get(section_name, key_name)
        else:
            return None

    def read_ini_int(self, section_name, key_name):
        key_name = key_name.lower()
        try:
            if section_name in self.config.sections() and key_name in self.config.options(section_name):
                return self.config.getint(section_name, key_name)
            else:
                return None
        except Exception as e:
            print(' Exception: ', e)
        return None


#
# 获取各类文件的存储路径
#
class conf_tool:
    conf = conf_ini(database_conf, "utf-8")

    def __init__(self):
        pass

    @staticmethod
    def get_config_file_name():
        return conf_tool.conf.read_ini_str("info", "config_file_name")

    @staticmethod
    def get_config_file_description():
        return conf_tool.conf.read_ini_str("info", "config_file_description")

    @staticmethod
    def get_config_file_path():
        return conf_tool.conf.read_ini_str("info", "config_file_path")

    @staticmethod
    # 存储数据文件的"根目录",程序中计算要用的数据文件均放在该路径定义的文件夹下面
    def get_base_path():
        return conf_tool.conf.read_ini_str("base", "base_dir")

    @staticmethod
    def get_etc_hosts_path():
        return conf_tool.conf.read_ini_str("host", "etc_hosts")

    @staticmethod
    def get_ltop_file_path():
        return conf_tool.conf.read_ini_str("metric", "ltop_file")
