# -*- coding: utf-8 -*-


import math

cpu_file_name = "data/monitor/aiocc.node.ca06_cpu_20180331031802.txt"
memory_file_name = "data/monitor/aiocc.node.ca06_memory_20180331031802.txt"
cpu_stat_option_index = {"user": 0, "nice": 1, "system": 2, "idle": 3, "iowait": 4, "irq": 5, "softirq": 6}
cpu_stat_option = ["user", "nice", "system", "idle", "iowait", "irq", "softirq"]

memory_stat_option_index = {"total": 0, "used": 1, "free": 2, "shared": 3, "buff/cache": 4, "available": 5,
                            "swap-total": 6,
                            "swap-used": 7, "swap-free": 8}
memory_stat_option = ["total", "used", "free", "shared", "buff/cache", "available", "swap-total", "swap-used",
                      "swap-free"]


def statistics_cpu(file_name, stat_option_index, stat_option):
    record_file = open(file_name, "r+")
    line_str = record_file.readline()
    last = list(map(int, line_str.strip('\n').split(" ")))
    line_str = record_file.readline()
    print(stat_option)
    while line_str:
        now = list(map(int, line_str.strip('\n').split(" ")))
        total = math.fsum(now) - math.fsum(last)
        rate = list(
            map(lambda x: (now[stat_option_index[x]] - last[stat_option_index[x]]) / total * 100, stat_option))
        print(100-rate[stat_option_index["idle"]])
        line_str = record_file.readline()
        last = now


def statistics_memory(file_name, stat_option_index, stat_option):
    record_file = open(file_name, "r+")
    line_str = record_file.readline()
    print(stat_option)
    while line_str:
        now = list(map(int, line_str.strip('\n').split(" ")))
        print(now)
        rate = list(map(lambda x: now[stat_option_index[x]] / now[stat_option_index["total"]] * 100 if x != "total" else now[stat_option_index["total"]], stat_option))
        print(rate)
        line_str = record_file.readline()


statistics_cpu(cpu_file_name, cpu_stat_option_index, cpu_stat_option)
# statistics_memory(memory_file_name, memory_stat_option_index, memory_stat_option)
