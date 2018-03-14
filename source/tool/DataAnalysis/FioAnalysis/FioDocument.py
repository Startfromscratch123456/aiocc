#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os


class FioDocument:
    def __init__(self):
        self.prefix = {'io': 0.0, 'bw': 0.0, 'iops': 0.0, 'runt': 0.0}
        self.clat = {'min': 0.0, 'max': 0.0, 'avg': 0.0, 'stdev': 0.0}
        self.clat_percentiles = {1.00: 0, 5.00: 0, 10.00: 0, 20.00: 0, 30.00: 0, 40.00: 0, 50.00: 0, 60.00: 0, 70.00: 0,
                                 80.00: 0, 90.00: 0, 95.00: 0, 99.00: 0, 99.50: 0, 99.90: 0, 99.95: 0, 99.99: 0}
        self.bw = {'min': 0.0, 'max': 0.0, 'per': 0.0, 'avg': 0.0, 'stdev': 0.0}
        self.all_jobs = {'io': 0.0, 'aggrb': 0.0, 'minb': 0.0, 'maxb': 0.0, 'mint': 0.0, 'maxt': 0.0}

    def getattribute(self, item):
        return self.__getattribute__(item)

    def init_data(self):
        self.__init__()

    def print_all_kv(self):
        print("prefix")
        print(self.prefix)
        print("clat")
        print(self.clat)
        print("clat_percentiles")
        print(self.clat_percentiles)
        print("bw")
        print(self.bw)
        print("all_jobs")
        print(self.all_jobs)


class FioRWDocument:
    def __init__(self, fio_read_document, fio_write_document):
        self.fio_read_document = fio_read_document
        self.fio_write_document = fio_write_document

    def __init__(self):
        self.test_parameter = {'direct': 0, 'iodepth': 0, 'rw': 'readwrite', 'ioengine': 'psync', 'bs': 0, 'size': 0,
                               'numjobs': 0, 'runtime': 0, 'name': ''}
        self.fio_read_document = None
        self.fio_write_document = None
        self.lat_usec = {2: 0.0, 4: 0.0, 10: 0.0, 20: 0.0, 50: 0.0, 100: 0.0, 250: 0.0, 500: 0.0, 750: 0.0, 1000: 0.0}
        self.lat_msec = {2: 0.0, 4: 0.0, 10: 0.0, 20: 0.0, 50: 0.0, 100: 0.0, 250: 0.0, 500: 0.0, 750: 0.0, 1000: 0.0,
                         2000: 0.0,
                         2001: 0.0}
        self.cpu = {'usr': 0.0, 'sys': 0.0, 'ctx': 0.0, 'majf': 0.0, 'minf': 0.0}
        self.io_depths = {1: 0.0, 2: 0.0, 4: 0.0, 8: 0.0, 16: 0.0, 32: 0.0, 64: 0.0, 65: 0.0}
        self.submit = {0: 0.0, 2: 0.0, 4: 0.0, 8: 0.0, 16: 0.0, 32: 0.0, 64: 0.0, 65: 0.0}
        self.complete = {0: 0.0, 2: 0.0, 4: 0.0, 8: 0.0, 16: 0.0, 32: 0.0, 64: 0.0, 65: 0.0}
        self.issued = {'total': 0, 'short': 0, 'drop': 0}
        self.latency = {'target': 0, 'window': 0, 'percentile': 0, 'depth': 0}

    def getattribute(self, item):
        return self.__getattribute__(item)

    def init_data(self):  # 不销毁对象  只清除数据  和__init__()函数不能混为一谈
        # self.test_parameter = {'direct': 0, 'iodepth': 0, 'rw': 'readwrite', 'ioengine': 'psync', 'bs': 0, 'size': 0,
        #                      'numjobs': 0, 'runtime': 0, 'name': ''}
        # 不能清理 这里的值一个fio文件才输出一个  后面很多操作要根据这里的参数进行例如读写方式，一旦清理，后面会失去操作依据
        self.fio_read_document = None
        self.fio_write_document = None
        self.lat_usec = {2: 0.0, 4: 0.0, 10: 0.0, 20: 0.0, 50: 0.0, 100: 0.0, 250: 0.0, 500: 0.0, 750: 0.0, 1000: 0.0}
        self.lat_msec = {2: 0.0, 4: 0.0, 10: 0.0, 20: 0.0, 50: 0.0, 100: 0.0, 250: 0.0, 500: 0.0, 750: 0.0, 1000: 0.0,
                         2000: 0.0, 2001: 0.0}
        self.cpu = {'usr': 0.0, 'sys': 0.0, 'ctx': 0.0, 'majf': 0.0, 'minf': 0.0}
        self.io_depths = {1: 0.0, 2: 0.0, 4: 0.0, 8: 0.0, 16: 0.0, 32: 0.0, 64: 0.0, 65: 0.0}
        self.submit = {0: 0.0, 2: 0.0, 4: 0.0, 8: 0.0, 16: 0.0, 32: 0.0, 64: 0.0, 65: 0.0}
        self.complete = {0: 0.0, 2: 0.0, 4: 0.0, 8: 0.0, 16: 0.0, 32: 0.0, 64: 0.0, 65: 0.0}
        self.issued = {'total': 0, 'short': 0, 'drop': 0}
        self.latency = {'target': 0, 'window': 0, 'percentile': 0, 'depth': 0}

    def print_all_kv(self):
        print("test_parameter")
        print(self.test_parameter)
        print("lat_usec")
        print(self.lat_usec)
        print("lat_msec")
        print(self.lat_msec)
        print("cpu")
        print(self.cpu)
        print("io_depths")
        print(self.io_depths)
        print("submit")
        print(self.submit)
        print("complete")
        print(self.complete)
        print("issued")
        print(self.issued)
        print("latency")
        print(self.latency)
