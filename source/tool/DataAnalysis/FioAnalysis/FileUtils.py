#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import re
from FioStatisticUtil import *


#
# 判断一行文件是否是有效的数据
#
def __is_legal(str):
    # 无用信息
    if "Laying out IO file(s)" in str:
        return False
    # 匹配命令行
    if "-group_reporting -name=" in str and "-ioengine=" in str:
        return True
    # aiocc_test: (groupid=0, jobs=10): err= 0: pid=16644: Sun Sep 11 00:50:53 2016
    if re.match(".+_test.*groupid=.*jobs=.*err=.*", str):
        return True
    # read : io=1709.3MB, bw=9717.8KB/s, iops=5268, runt=180116msec
    if re.match(".*io=.*bw=.*iops=.*runt=.*", str) and "read" in str or "write" in str:
        return True
    # clat (usec): min=2, max=2765.2K, avg=3305.67, stdev=30529.75
    if re.match(".*lat *(.sec).*min=.*max=.*avg=.*stdev=.*", str):
        return True
    # 匹配|  1.00th=[   62],  5.00th=[   72], 10.00th=[   78], 20.00th=[   85],
    if "|" in str and "th=[" in str and "]" in str:
        return True
    # clat percentiles (usec):
    if re.match(".*clat percentiles.*([um]sec).*", str):
        return True
    #
    # lat(usec): 4 = 0.01 %, 10 = 0.01 %, 20 = 0.01 %, 50 = 0.01 %, 100 = 41.43 %
    # lat(usec): 250 = 51.46 %, 500 = 0.45 %, 750 = 0.08 %, 1000 = 0.08 %
    # lat(msec): 2 = 0.54 %, 4 = 1.21 %, 10 = 1.26 %, 20 = 0.30 %, 50 = 1.26 %
    # lat(msec): 100 = 1.07 %, 250 = 0.70 %, 500 = 0.07 %, 750 = 0.03 %, 1000 = 0.02 %
    # lat(msec): 2000 = 0.02 %, >= 2000 = 0.01 %
    #
    if re.match(".*lat ([mu]sec).*: \d*=\d*.*%, ", str):
        return True
    if re.match(".*cpu.*: usr=\d*.*%, sys=\d*.*%, ctx=\d*.*%, majf=.*, minf=.*", str):
        return True
    if re.match(".*IO depths.*:.*%", str):
        return True
    if re.match(".*submit.*:.*%", str):
        return True
    if re.match(".*complete.*:.*%", str):
        return True
    if re.match(".*issued.*:.*%.*total=.*short.*drop=.*", str):
        return True
    if re.match(".*latency.*:.*target=.*window=.*percentile=.*depth=.*", str):
        return True
    if re.match(".*Run status group.*(all jobs).*", str):
        return True
    if re.match(".*io=.*, aggrb=.*, minb=.*, maxb=.*, mint=.*, maxt=.*", str) and "READ" in str or "WRITE" in str:
        return True
    # 空格
    if re.match("[ \t\n\x0B\f\r]+", str):
        return True
    return False


def get_all_fio_test_files(root_path):
    all_files = []
    for workload_dir in os.listdir(root_path):
        workload_dir = root_path + os.sep + workload_dir
        for each_dir in os.listdir(workload_dir):
            if "20180313121137" not in each_dir:
                continue
            each_dir = workload_dir + os.sep + each_dir
            for rw_dir in os.listdir(each_dir):
                rw_dir = each_dir + os.sep + rw_dir
                for file_name in os.listdir(rw_dir):
                    file_name = rw_dir + os.path.sep + file_name
                    all_files.append(file_name)
    return all_files


# handle the test result of fio,filtering out extraneous information
def fio_filter(root_path):
    for file_name in get_all_fio_test_files(root_path):
        fio_file = open(file_name, "r+")
        line_str = fio_file.readline()
        fio_file_content = ""
        count = 0
        while line_str:
            if __is_legal(line_str):
                fio_file_content += line_str
                count += 1
            line_str = fio_file.readline()

        fio_file.close()
        fio_file = open(file_name, "w+")
        fio_file.write(fio_file_content)
        fio_file.close()

        if count < 10:
            print("不完整的文件：" + file_name)


def fio_file_to_document(file_name):
    fio_file = open(file_name, "r")
    line_str = fio_file.readline()
    fio_file_pattern = re.search("-rw={1}[a-z]*", line_str).group(0)
    fio_file_pattern = fio_file_pattern[fio_file_pattern.index("=") + 1:]

    operator = {'read': fio_rfile_to_document, 'write': fio_wfile_to_document, 'readwrite': fio_rwfile_to_document,
                'randread': fio_rdrfile_to_document, 'randwrite': fio_rdwfile_to_document,
                'randrw': fio_rdrwfile_to_document}
    fio_file.close()
    return operator.get(fio_file_pattern)(file_name)


def fio_rfile_to_document(file_name):
    return _fio_file_to_document(file_name)


def fio_rdrfile_to_document(file_name):
    return _fio_file_to_document(file_name)


def fio_wfile_to_document(file_name):
    return _fio_file_to_document(file_name)


def fio_rdwfile_to_document(file_name):
    return _fio_file_to_document(file_name)


def fio_rwfile_to_document(file_name):
    return _fio_rwfile_to_document(file_name)


def fio_rdrwfile_to_document(file_name):
    return _fio_rwfile_to_document(file_name)


def _fio_file_to_document(file_name):
    fio_file = open(file_name, "r")
    line_str = fio_file.readline()  # the first line
    fio_file_pattern = re.search("-rw={1}[a-z]*", line_str).group(0)
    fio_file_pattern = fio_file_pattern[fio_file_pattern.index("=") + 1:]
    avg_fio_document = AvgFioDocument()

    fio_document = FioDocument()
    fio_rwdocument = FioRWDocument()
    avg_fio_document.avg_fio_document.test_parameter['rw'] = fio_rwdocument.test_parameter['rw'] = fio_file_pattern
    rs_str = re.findall(r"[a-zA-Z]+=\d+\.*\d*", line_str)
    for index in range(len(rs_str)):
        avg_fio_document.avg_fio_document.test_parameter[rs_str[index][0:rs_str[index].index("=")]] = \
            fio_rwdocument.test_parameter[rs_str[index][0:rs_str[index].index("=")]] = int(rs_str[index][
                                                                                           rs_str[index].index(
                                                                                               "=") + 1:])
    # for rw
    rs_str = re.findall(r"[a-zA-Z]+=[a-zA-Z]+", line_str)
    for index in range(len(rs_str)):
        avg_fio_document.avg_fio_document.test_parameter[rs_str[index][0:rs_str[index].index("=")]] = \
            fio_rwdocument.test_parameter[rs_str[index][0:rs_str[index].index("=")]] = (rs_str[index][
                                                                                        rs_str[index].index("=") + 1:])
    # iodepth
    rs_str = re.findall(r"iodepth[ ]*[\d+\.*\d*]+", line_str)
    for index in range(len(rs_str)):
        avg_fio_document.avg_fio_document.test_parameter[rs_str[index][0:rs_str[index].index(" ")]] = \
            fio_rwdocument.test_parameter[rs_str[index][0:rs_str[index].index(" ")]] = (rs_str[index][
                                                                                        rs_str[index].index(" ") + 1:])

    line_str = fio_file.readline()  # aiocc_test: (groupid=0
    while line_str and "(groupid=" not in line_str:
        line_str = fio_file.readline()
    while line_str and "(groupid=" in line_str:
        # filtering out read : io=2795.8MB, bw=28627KB/s, iops=28627, runt=100004msec
        line_str = fio_file.readline()
        rs_str = re.findall(r"[a-zA-Z]+=\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_document.prefix[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                                   rs_str[index].index("=") + 1:])
        if not 'KB/s' in line_str:
            fio_document.prefix['bw'] /= 1000
        # filtering out clat (usec): min=10, max=2191.2K, avg=48.32, stdev=2735.96
        line_str = fio_file.readline()
        if "slat" in line_str:  # skip slat (usec): min=0, max=2075, avg= 9.41, stdev=19.30
            line_str = fio_file.readline()
        rate = 1
        if "msec" in line_str:
            rate = 1000
        rs_str = re.findall(r"[a-zA-Z]+=\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_document.clat[rs_str[index][0:rs_str[index].index("=")]] = rate * float(rs_str[index][
                                                                                        rs_str[index].index("=") + 1:])

        # lat (usec): min=10, max=2191.2K, avg=48.56, stdev=2736.01
        # clat percentiles(usec):
        line_str = fio_file.readline()
        line_str = fio_file.readline()
        rate = 1
        if "msec" in line_str:
            rate = 1000
        # filtering out   clat percentiles (usec):
        line_str = fio_file.readline() + fio_file.readline() + fio_file.readline() + fio_file.readline() + fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 1, 2):
            fio_document.clat_percentiles[float(rs_str[index])] = rate * int(rs_str[index + 1])

        # filtering out    bw (KB  /s): min=    0, max=54394, per=12.32%, avg=9345.00, stdev=8587.24
        line_str = fio_file.readline()
        rs_str = re.findall(r"[a-zA-Z]+= *\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_document.bw[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                               rs_str[index].index("=") + 1:])

        # filtering out   lat (usec)
        line_str = ""
        tem_line_str = fio_file.readline()
        if "usec" in tem_line_str:  # 有的时延比较大的情况一开始就是以毫秒为单位
            while "lat (usec)" in tem_line_str:
                line_str += tem_line_str
                tem_line_str = fio_file.readline()
            rs_str = re.findall(r"\d+\.*\d*", line_str)
            for index in range(0, len(rs_str) - 1, 2):
                fio_rwdocument.lat_usec[int(rs_str[index])] = float(rs_str[index + 1])
                # 处理某些点没有值的情况,就用前一个代替
            lat_xsec_keys = list(fio_rwdocument.lat_usec.keys())
            lat_xsec_keys.sort()
            for index in range(0, len(lat_xsec_keys) - 1, 1):
                if fio_rwdocument.lat_usec[lat_xsec_keys[index + 1]] == 0:
                    fio_rwdocument.lat_usec[lat_xsec_keys[index + 1]] = fio_rwdocument.lat_usec[
                        lat_xsec_keys[index]]

        # filtering out   lat (msec)
        line_str = ""
        if "msec" in tem_line_str:
            while "lat (msec)" in tem_line_str:
                line_str += tem_line_str
                tem_line_str = fio_file.readline()

            rs_str = re.findall(r"\d+\.*\d*", line_str)
            for index in range(0, len(rs_str) - 2, 2):
                fio_rwdocument.lat_msec[int(rs_str[index])] = float(rs_str[index + 1])
            if ">=2000=" in line_str:  # 标记是否有>=2000的出现
                fio_rwdocument.lat_msec[2001] = float(rs_str[len(rs_str) - 1])

            lat_xsec_keys = list(fio_rwdocument.lat_msec.keys())
            lat_xsec_keys.sort()
            for index in range(0, len(lat_xsec_keys) - 1, 1):
                if fio_rwdocument.lat_msec[lat_xsec_keys[index + 1]] == 0:
                    fio_rwdocument.lat_msec[lat_xsec_keys[index + 1]] = fio_rwdocument.lat_msec[
                        lat_xsec_keys[index]]
        # cpu    : usr=1.85%, sys=19.18%, ctx=128921, majf=0, minf=12
        line_str = tem_line_str
        rs_str = re.findall(r"[a-zA-Z]+= *\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_rwdocument.cpu[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                                  rs_str[index].index("=") + 1:])

        # IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
        line_str = fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 2, 2):
            fio_rwdocument.io_depths[int(rs_str[index])] = float(rs_str[index + 1])
        fio_rwdocument.io_depths[65] = float(rs_str[len(rs_str) - 1])

        # submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
        line_str = fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 2, 2):
            fio_rwdocument.submit[int(rs_str[index])] = float(rs_str[index + 1])
        fio_rwdocument.submit[65] = float(rs_str[len(rs_str) - 1])

        # complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
        line_str = fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 2, 2):
            fio_rwdocument.complete[int(rs_str[index])] = float(rs_str[index + 1])
        fio_rwdocument.complete[65] = float(rs_str[len(rs_str) - 1])

        # issued    : total=r=7587061/w=0/d=0, short=r=0/w=0/d=0, drop=r=0/w=0/d=0
        line_str = fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        fio_rwdocument.issued['total'] = int(rs_str[0])
        fio_rwdocument.issued['short'] = int(rs_str[3])
        fio_rwdocument.issued['drop'] = int(rs_str[6])

        # latency   : target=0, window=0, percentile=100.00%, depth=5
        line_str = fio_file.readline()
        rs_str = re.findall(r"[a-zA-Z]+= *\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_rwdocument.latency[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                                      rs_str[index].index("=") + 1:])
        # all_jobs
        fio_file.readline()
        fio_file.readline()
        line_str = fio_file.readline()
        rs_str = re.findall(r"[a-zA-Z]+= *\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_document.all_jobs[rs_str[index][0:rs_str[index].index("=")]] = float(
                rs_str[index][rs_str[index].index("=") + 1:])
        if not 'KB/s' in line_str:
            fio_document.prefix['bw'] /= 1000
        while line_str and "(groupid=" not in line_str:
            line_str = fio_file.readline()

        if "read" in fio_rwdocument.test_parameter['rw']:
            fio_rwdocument.fio_read_document = fio_document
            fio_rwdocument.fio_write_document = None
        else:
            fio_rwdocument.fio_read_document = None
            fio_rwdocument.fio_write_document = fio_document
        avg_fio_document.add_fio_document(fio_rwdocument)

        fio_rwdocument.init_data()
        fio_document.init_data()
    if "read" in fio_rwdocument.test_parameter['rw']:
        avg_fio_document.avg_fio_document.fio_write_document = None
    else:
        avg_fio_document.avg_fio_document.fio_read_document = None
    fio_file.close()
    return avg_fio_document.get_avg_fio_document()


def _fio_rwfile_to_document(file_name):
    fio_file = open(file_name, "r")
    line_str = fio_file.readline()  # the first line
    fio_file_pattern = re.search("-rw={1}[a-z]*", line_str).group(0)
    fio_file_pattern = fio_file_pattern[fio_file_pattern.index("=") + 1:]

    avg_fio_document = AvgFioDocument()
    fio_rdocument = FioDocument()
    fio_wdocument = FioDocument()
    fio_rwdocument = FioRWDocument()

    avg_fio_document.avg_fio_document.test_parameter['rw'] = fio_rwdocument.test_parameter['rw'] = fio_file_pattern
    rs_str = re.findall(r"[a-zA-Z]+=\d+\.*\d*", line_str)
    for index in range(len(rs_str)):
        avg_fio_document.avg_fio_document.test_parameter[rs_str[index][0:rs_str[index].index("=")]] = \
            fio_rwdocument.test_parameter[rs_str[index][0:rs_str[index].index("=")]] = int(
            rs_str[index][rs_str[index].index("=") + 1:])

    # for rw
    rs_str = re.findall(r"[a-zA-Z]+=[a-zA-Z]+", line_str)
    for index in range(len(rs_str)):
        avg_fio_document.avg_fio_document.test_parameter[rs_str[index][0:rs_str[index].index("=")]] = \
            fio_rwdocument.test_parameter[rs_str[index][0:rs_str[index].index("=")]] = (rs_str[index][
                                                                                        rs_str[index].index("=") + 1:])
    # iodepth
    rs_str = re.findall(r"iodepth[ ]*[\d+\.*\d*]+", line_str)
    for index in range(len(rs_str)):
        avg_fio_document.avg_fio_document.test_parameter[rs_str[index][0:rs_str[index].index(" ")]] = \
            fio_rwdocument.test_parameter[rs_str[index][0:rs_str[index].index(" ")]] = (rs_str[index][
                                                                                        rs_str[index].index(" ") + 1:])

    line_str = fio_file.readline()  # aiocc_test: (groupid=0
    while line_str and "(groupid=" not in line_str:
        line_str = fio_file.readline()
    while line_str and "(groupid=" in line_str:

        '''
            ---------------------- read part-------------------------
        '''
        # filtering out read : io=2795.8MB, bw=28627KB/s, iops=28627, runt=100004msec
        line_str = fio_file.readline()
        rs_str = re.findall(r"[a-zA-Z]+=\d+\.*\d*", line_str)

        for index in range(len(rs_str)):
            fio_rdocument.prefix[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                                    rs_str[index].index("=") + 1:])
        # the unit problem
        if len(re.findall(r"io=\d+\.*\d*KB", line_str)) != 0:
            fio_rdocument.prefix['io'] /= 1024
        if len(re.findall(r"bw=\d+\.*\d*B/s", line_str)) != 0:
            fio_rdocument.prefix['bw'] /= 1024
        if not 'KB/s' in line_str:
            fio_rdocument.prefix['bw'] /= 1000
        # filtering out clat (usec): min=10, max=2191.2K, avg=48.32, stdev=2735.96
        line_str = fio_file.readline()
        if "slat" in line_str:  # skip slat (usec): min=0, max=2075, avg= 9.41, stdev=19.30
            line_str = fio_file.readline()
        rate = 1
        if "msec" in line_str:
            rate = 1000
        rs_str = re.findall(r"[a-zA-Z]+=\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_rdocument.clat[rs_str[index][0:rs_str[index].index("=")]] = rate * float(rs_str[index][
                                                                                         rs_str[index].index("=") + 1:])

        # lat (usec): min=10, max=2191.2K, avg=48.56, stdev=2736.01
        # clat percentiles(usec):
        line_str = fio_file.readline()
        line_str = fio_file.readline()
        rate = 1
        if "msec" in line_str:
            rate = 1000
        # filtering out   clat percentiles (usec):
        line_str = fio_file.readline() + fio_file.readline() + fio_file.readline() + fio_file.readline() + fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 1, 2):
            fio_rdocument.clat_percentiles[float(rs_str[index])] = rate * int(rs_str[index + 1])

        '''
                           ---------------------- write part-------------------------
        '''

        # filtering out read : io=2795.8MB, bw=28627KB/s, iops=28627, runt=100004msec
        line_str = fio_file.readline()
        rs_str = re.findall(r"[a-zA-Z]+=\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_wdocument.prefix[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                                    rs_str[index].index("=") + 1:])

        # the unit problem
        if len(re.findall(r"io=\d+\.*\d*KB", line_str)) != 0:
            fio_wdocument.prefix['io'] /= 1024
        if len(re.findall(r"bw=\d+\.*\d*B/s", line_str)) != 0:
            fio_wdocument.prefix['bw'] /= 1024
        if not 'KB/s' in line_str:
            fio_wdocument.prefix['bw'] /= 1000
        # filtering out clat (usec): min=10, max=2191.2K, avg=48.32, stdev=2735.96
        line_str = fio_file.readline()
        if "slat" in line_str:  # skip slat (usec): min=0, max=2075, avg= 9.41, stdev=19.30
            line_str = fio_file.readline()
        rs_str = re.findall(r"[a-zA-Z]+=\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_wdocument.clat[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                                  rs_str[index].index("=") + 1:])

        # lat (usec): min=10, max=2191.2K, avg=48.56, stdev=2736.01
        # clat percentiles(usec):
        fio_file.readline()
        fio_file.readline()
        # filtering out   clat percentiles (usec):
        line_str = fio_file.readline() + fio_file.readline() + fio_file.readline() + fio_file.readline() + fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 1, 2):
            fio_wdocument.clat_percentiles[float(rs_str[index])] = int(rs_str[index + 1])

        '''
                          ---------------------- rw part-------------------------
       '''
        # filtering out   lat (usec)
        line_str = ""
        tem_line_str = fio_file.readline()
        if "usec" in tem_line_str:  # 有的时延比较大的情况一开始就是以毫秒为单位
            while "lat (usec)" in tem_line_str:
                line_str += tem_line_str
                tem_line_str = fio_file.readline()
            rs_str = re.findall(r"\d+\.*\d*", line_str)
            for index in range(0, len(rs_str) - 1, 2):
                fio_rwdocument.lat_usec[int(rs_str[index])] = float(rs_str[index + 1])
                # 处理某些点没有值的情况,就用前一个代替
            lat_xsec_keys = list(fio_rwdocument.lat_usec.keys())
            lat_xsec_keys.sort()
            for index in range(0, len(lat_xsec_keys) - 1, 1):
                if (fio_rwdocument.lat_usec[lat_xsec_keys[index + 1]] == 0):
                    fio_rwdocument.lat_usec[lat_xsec_keys[index + 1]] = fio_rwdocument.lat_usec[
                        lat_xsec_keys[index]]

        # filtering out   lat (msec)
        line_str = ""
        if "msec" in tem_line_str:
            while "lat (msec)" in tem_line_str:
                line_str += tem_line_str
                tem_line_str = fio_file.readline()

            rs_str = re.findall(r"\d+\.*\d*", line_str)
            for index in range(0, len(rs_str) - 2, 2):
                fio_rwdocument.lat_msec[int(rs_str[index])] = float(rs_str[index + 1])
            if ">=2000=" in line_str:  # 标记是否有>=2000的出现
                fio_rwdocument.lat_msec[2001] = float(rs_str[len(rs_str) - 1])

            lat_xsec_keys = list(fio_rwdocument.lat_msec.keys())
            lat_xsec_keys.sort()
            for index in range(0, len(lat_xsec_keys) - 1, 1):
                if fio_rwdocument.lat_msec[lat_xsec_keys[index + 1]] == 0:
                    fio_rwdocument.lat_msec[lat_xsec_keys[index + 1]] = fio_rwdocument.lat_msec[
                        lat_xsec_keys[index]]

        # cpu    : usr=1.85%, sys=19.18%, ctx=128921, majf=0, minf=12
        line_str = tem_line_str
        rs_str = re.findall(r"[a-zA-Z]+= *\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_rwdocument.cpu[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                                  rs_str[index].index("=") + 1:])

        # IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
        line_str = fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 1, 2):
            fio_rwdocument.io_depths[int(rs_str[index])] = float(rs_str[index + 1])
        fio_rwdocument.io_depths[65] = float(rs_str[len(rs_str) - 1])

        # submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
        line_str = fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 2, 2):
            fio_rwdocument.submit[int(rs_str[index])] = float(rs_str[index + 1])
        fio_rwdocument.submit[65] = float(rs_str[len(rs_str) - 1])

        # complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
        line_str = fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        for index in range(0, len(rs_str) - 2, 2):
            fio_rwdocument.complete[int(rs_str[index])] = float(rs_str[index + 1])
        fio_rwdocument.complete[65] = float(rs_str[len(rs_str) - 1])

        # issued    : total=r=7587061/w=0/d=0, short=r=0/w=0/d=0, drop=r=0/w=0/d=0
        line_str = fio_file.readline()
        rs_str = re.findall(r"\d+\.*\d*", line_str)
        fio_rwdocument.issued['total'] = int(rs_str[0])
        fio_rwdocument.issued['short'] = int(rs_str[3])
        fio_rwdocument.issued['drop'] = int(rs_str[6])

        # latency   : target=0, window=0, percentile=100.00%, depth=5
        line_str = fio_file.readline()
        rs_str = re.findall(r"[a-zA-Z]+= *\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_rwdocument.latency[rs_str[index][0:rs_str[index].index("=")]] = float(rs_str[index][
                                                                                      rs_str[index].index("=") + 1:])
        # all_jobs
        fio_file.readline()
        fio_file.readline()
        line_str = fio_file.readline()

        # READ
        rs_str = re.findall(r"[a-zA-Z]+= *\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_rdocument.all_jobs[rs_str[index][0:rs_str[index].index("=")]] = float(
                rs_str[index][rs_str[index].index("=") + 1:])

        # WRITE
        rs_str = re.findall(r"[a-zA-Z]+= *\d+\.*\d*", line_str)
        for index in range(len(rs_str)):
            fio_wdocument.all_jobs[rs_str[index][0:rs_str[index].index("=")]] = float(
                rs_str[index][rs_str[index].index("=") + 1:])

        while line_str and "(groupid=" not in line_str:
            line_str = fio_file.readline()

        fio_rwdocument.fio_read_document = fio_rdocument
        fio_rwdocument.fio_write_document = fio_wdocument
        avg_fio_document.add_fio_document(fio_rwdocument)

        fio_rwdocument.init_data()
        fio_rdocument.init_data()
        fio_wdocument.init_data()
    fio_file.close()
    return avg_fio_document.get_avg_fio_document()
