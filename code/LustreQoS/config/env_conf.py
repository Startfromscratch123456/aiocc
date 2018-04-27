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
import socket
import logging
import glob
from typing import List
from tensorlib import LustreCommon
from tensorlib import common


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
                        raise RuntimeError("config file maybe not exist.")
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


nodeid_map = {
    'node11.aiocc': 1,
    'node12.aiocc': 2,
    'node13.aiocc': 3,
    'node14.aiocc': 4,
    'node15.aiocc': 5,
    'node16.aiocc': 6,
    'node17.aiocc': 7,
}

TICK_LEN = 1

servers = [
    'node11.aiocc',
    'node12.aiocc',
    'node13.aiocc',
    'node14.aiocc',
    'node15.aiocc',
]

PI_PER_CLIENT_OBD = 11
OBD_PER_CLIENT_MA = len(servers)

clients = [
    'node16.aiocc',
    'node17.aiocc',
]

# control_method can be 'mrif', 'rules', or 'mrif_tau'
control_method = 'mrif_tau'
rule_archive_path = '/home/development/AIOCC/source/aiocc/capes/capes-oss/rules'

if control_method == 'mrif':
    cpv_spec = [
        # name, initial value, min, max, step size
        ['mrif', 8, 1, 256, 4],
    ]
elif control_method == 'rules':
    cpv_spec = [
        # name, initial value, min, max, step size
        ['rtt_ratio_changepoint', 237, 37, 437, 10],
        ['ewma_changepoint_a', 41001, 20001, 60001, 1000],
        ['ewma_changepoint_b', 48427, 28427, 68427, 1000],
    ]
    rule_template_file = os.path.join(rule_archive_path, 'iorcp_alpha9999_472_3cpvs.csv')
    with open(rule_template_file, 'r') as f:
        rule_template = f.read()
elif control_method == 'mrif_tau':
    cpv_spec = [
        # name, initial value, min, max, step size
        ['mrif', 8, 1, 256, 4],
        ['tau', 32840, 0, 400000, 1500],
    ]
    rule_template_file = os.path.join(rule_archive_path, 'tau_only.csv')
    with open(rule_template_file, 'r') as f:
        tau_only_rule_template = f.read()
    _tau = 0
else:
    raise RuntimeError('Unknown control method: ' + control_method)

my_hostname = socket.gethostname()


def collect_ping_to_servers() -> List[float]:
    result = []
    for srv in servers:
        # PI 7
        result.append(common.get_ping_time(srv))
    return result


def collect_osc_pi_cpv(osc_path: str) -> List[float]:
    result = list()
    # PI 0
    result.append(LustreCommon.read_proc_file(os.path.join(osc_path, 'max_rpcs_in_flight')))
    # PI 1
    result.append(LustreCommon.read_proc_file(os.path.join(osc_path, 'min_brw_rpc_gap')))

    with open(os.path.join(osc_path, 'import'), 'r') as importfile:
        # import is a proc file and should be read as a whole, i.e., not using readline()
        import_data = importfile.read()

    # PI 2
    result.append(LustreCommon.extract_ack_ewma_from_import(import_data))
    # PI 3
    result.append(LustreCommon.extract_sent_ewma_from_import(import_data))
    # PI 4
    result.append(LustreCommon.extract_rtt_ratio100_from_import(import_data))
    # PI 5
    result.append(LustreCommon.extract_read_bandwidth_from_import(import_data))
    # PI 6
    result.append(LustreCommon.extract_write_bandwidth_from_import(import_data))
    # PI 7
    result.append(_tau)
    # PI 8
    result.append(LustreCommon.read_proc_file(os.path.join(osc_path, 'cur_dirty_bytes')))
    # PI 9
    result.append(LustreCommon.read_proc_file(os.path.join(osc_path, 'max_dirty_mb')))

    return result


def lustre_collect_pi() -> List[float]:
    if my_hostname in clients:
        # collect ping time to all servers
        ping_times = collect_ping_to_servers()
        # collect PIs
        oscs = glob.glob('/proc/fs/lustre/osc/*/import')
        osc_paths = [os.path.dirname(p) for p in oscs]
        osc_paths.sort()
        osc_pis = list()
        for osc in osc_paths:
            osc_pis.extend(collect_osc_pi_cpv(osc))

        result = osc_pis + ping_times
        assert len(result) == len(servers) * PI_PER_CLIENT_OBD
        return result
    elif my_hostname in servers:
        return list()
    else:
        raise RuntimeError('My hostname is neither client or server')


def lustre_controller(cpvs: List[float]):
    """Perform an action

    Applies the CPVs to the system

    :param cpvs: the new values of CPVs
    :return:
    """
    # cpvs[0] is the action id, remove it
    cpvs = cpvs[1:]
    if my_hostname in clients:
        if control_method == 'mrif':
            mrif = int(cpvs[0])
            assert cpv_spec[0][2] <= mrif <= cpv_spec[0][3]
            LustreCommon.set_mrif(mrif, len(servers))
        elif control_method == 'rules':
            # convert cpvs list to a dict
            kv = dict([('cpv{0}'.format(i+1), int(cpv)) for i, cpv in enumerate(cpvs)])
            rule = LustreCommon.gen_rule(rule_template, kv)
            LustreCommon.set_rule(rule, len(servers))
        elif control_method == 'mrif_tau':
            global _tau
            mrif = int(cpvs[0])
            assert cpv_spec[0][2] <= mrif <= cpv_spec[0][3]
            LustreCommon.set_mrif(mrif, len(servers))

            _tau = int(cpvs[1])
            rule = LustreCommon.gen_rule(tau_only_rule_template, {'tau': _tau})
            LustreCommon.set_rule(rule, len(servers))
        else:
            raise RuntimeError('Unknown control method: ' + control_method)
    elif my_hostname in servers:
        pass
    else:
        raise RuntimeError('My hostname is neither client or server')


opt = {
    'loglevel': logging.INFO,
    'log_lazy_flush': True,
    # level 1 enables cProfile, level 2 also enables Pympler
    'ma_debugging_level': 0,
    'dqldaemon_debugging_level': 0,
    'dbfile': '/home/development/AIOCC/aiocc/database/ascar_replay_db.sqlite',
    'tick_len': TICK_LEN,                   # duration of a tick in second
    'ticks_per_observation': 10,            # how many ticks are in an observation
    'nodeid_map': nodeid_map,
    'clients': clients,
    'servers': servers,
    'cpvs': cpv_spec,
    'num_actions': 2 * len(cpv_spec) + 1,
    'pi_per_client_obd': PI_PER_CLIENT_OBD,
    'obd_per_client_ma': OBD_PER_CLIENT_MA,
    'tick_data_size': PI_PER_CLIENT_OBD * OBD_PER_CLIENT_MA * len(clients),  # only four clients have PI so far

    # Collectors are functions that collect PIs. MA calls them in order and concatenate
    # their returns into a single list before passing them to IntfDaemon
    'collectors': [lustre_collect_pi],
    'controller': lustre_controller,
    'intf_daemon_loc': '192.168.3.181:9123',
    'ascar.MonitorAgent.MonitorAgent_logfile': '/home/development/AIOCC/source/aiocc/log/ma_log.txt',
    'ascar.IntfDaemon.IntfDaemon_logfile': '/home/development/AIOCC/source/aiocc/log/intfdaemon_log.txt',
    'ascar.DQLDaemon.DQLDaemon_logfile': '/home/development/AIOCC/source/aiocc/log/dqldaemon_log.txt',
    'pidfile_dir': '/tmp',

    'start_random_rate': 1,
    # How many actions should the exploration period include
    'exploration_period': 1000000,
    'random_action_probability': 0.05,

    'minibatch_size': 32,
    'enable_tuning': True,
}
