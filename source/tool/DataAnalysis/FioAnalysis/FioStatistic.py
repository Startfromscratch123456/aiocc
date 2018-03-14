#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import re
import numpy as np
import matplotlib.pyplot as plt
from FileUtils import *
from PrintUtil import print_fio_document
from FioDocument import *


class FioDocumentStatistic:
    def __init__(self):
        self.parameter_a = 'tbf'
        self.parameter_b = 'sscdt'
        self.documents = {'read': {self.parameter_a: [], self.parameter_b: []},
                          'randread': {self.parameter_a: [], self.parameter_b: []},
                          'write': {self.parameter_a: [], self.parameter_b: []},
                          'randwrite': {self.parameter_a: [], self.parameter_b: []},
                          'readwrite': {self.parameter_a: [], self.parameter_b: []},
                          'randrw': {self.parameter_a: [], self.parameter_b: []}}

    def __load_data__(self, rootpath):
        for io_pattern in os.listdir(rootpath):
            for filename in os.listdir(rootpath + os.sep + io_pattern):
                path = (rootpath + os.sep + io_pattern + os.sep + filename)
                if self.parameter_a in filename:
                    self.documents[io_pattern][self.parameter_a].append(fio_file_to_document(path))
                elif self.parameter_b in filename:
                    self.documents[io_pattern][self.parameter_b].append(fio_file_to_document(path))

    def load_data(self, rootpath):
        self.__load_data__(rootpath)

    def get_single_rwcomparison_bw(self, io_pattern, property_name):

        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        plt.figure(1, figsize=(9, 6))
        subplot = plt.subplot(1, 1, 1)

        width = 0.3

        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b))
        subplot.set_title("comparison of " + str(property_name))
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('throughput (KB/S)')
        plt.grid(True)
        plt.savefig('images' + os.sep + io_pattern + os.sep + property_name + '.png')
        plt.close()

    def get_single_rwcomparison_io(self, io_pattern, property_name='io'):

        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        plt.figure(1, figsize=(6, 4.5))
        subplot = plt.subplot(1, 1, 1)
        plt.grid(True)
        width = 0.3

        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='center')
        subplot.set_title("comparison of " + str(property_name))
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('IO (MB)')

        plt.savefig('images' + os.sep + io_pattern + os.sep + property_name + '.png')
        plt.close()

    def get_single_rwcomparison_iops(self, io_pattern, property_name='iops'):

        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        plt.figure(1, figsize=(9, 6))
        subplot = plt.subplot(1, 1, 1)
        plt.grid(True)
        width = 0.3

        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b))
        subplot.set_title("comparison of " + str(property_name))
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('IOPS ')

        plt.savefig('images' + os.sep + io_pattern + os.sep + property_name + '.png')
        plt.close()

    def get_rwcomparison_bw(self, io_pattern, property_name):

        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'

        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        plt.figure(1, figsize=(9, 6))
        plt.grid(True)
        subplot = plt.subplot(2, 1, 1)

        width = 0.3

        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b))
        subplot.set_title("comparison of " + str(property_name) + "（read）")
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('throughput (KB/S)')

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []
        fio_document = 'fio_write_document'

        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        subplot = plt.subplot(2, 1, 2)
        width = 0.3
        plt.grid(True)

        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b))
        subplot.set_title("comparison of " + str(property_name) + "（write）")
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('throughput (KB/S)')

        plt.savefig('images' + os.sep + io_pattern + os.sep + property_name + '.png')
        plt.close()

    def get_rwcomparison_io(self, io_pattern, property_name='io'):

        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'

        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        plt.figure(1, figsize=(9, 6))
        plt.grid(True)
        subplot = plt.subplot(2, 1, 1)
        width = 0.3

        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='center')
        subplot.set_title("comparison of " + str(property_name) + "(read)")
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('IO (MB)')

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_write_document'

        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        subplot = plt.subplot(2, 1, 2)
        width = 0.3
        plt.grid(True)
        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='center')
        subplot.set_title("comparison of " + str(property_name) + "(write)")
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('IO (MB)')

        plt.savefig('images' + os.sep + io_pattern + os.sep + property_name + '.png')
        plt.close()

    def get_rwcomparison_iops(self, io_pattern, property_name='iops'):

        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        plt.figure(1, figsize=(9, 6))
        subplot = plt.subplot(2, 1, 1)
        plt.grid(True)
        width = 0.3

        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b))
        subplot.set_title("comparison of " + str(property_name) + "(read)")
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('IOPS ')

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_write_document'

        # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).prefix[property_name]))

        subplot = plt.subplot(2, 1, 2)
        plt.grid(True)
        width = 0.3
        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b))
        subplot.set_title("comparison of " + str(property_name) + "(write)")
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel('IOPS ')

        plt.savefig('images' + os.sep + io_pattern + os.sep + property_name + '.png')
        plt.close()

    def get_single_rwcomparison_clat(self, io_pattern, property_name, ylabel='complete latency(us)'):

        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

            # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).clat[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).clat[property_name]))

        plt.figure(1, figsize=(9, 6))
        subplot = plt.subplot(1, 1, 1)
        width = 0.3
        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='center')
        subplot.set_title("comparison of " + str(property_name))
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel(ylabel)
        plt.grid(True)
        plt.savefig('images' + os.sep + io_pattern + os.sep + '_clat_' + property_name + '.png')
        plt.close()

        # lower  95%

    def get_single_rwcomparison_clatA(self, io_pattern='read', bs=1, limit=12):

        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_a_var = None
        for ele in parameter_a_list:
            if (int(ele.test_parameter['bs']) == int(bs)):
                parameter_a_var = ele
                break
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_b_var = None
        for ele in parameter_b_list:
            if (int(ele.test_parameter['bs']) == int(bs)):
                parameter_b_var = ele
                break
        x_coordinate = np.arange(1, 1 + limit, 1)
        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

        parameter_a_clat = sorted(parameter_a_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])
        parameter_b_clat = sorted(parameter_b_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])

        # parameter_a
        for v in parameter_a_clat:
            y_coordinate_parameter_a.append(float(v[1]))
            x_label.append(v[0])

        # parameter_b_var
        for v in parameter_b_clat:
            y_coordinate_parameter_b.append(float(v[1]))

        plt.figure(1, figsize=(9, 6))
        subplot = plt.subplot(1, 1, 1)
        width = 0.3
        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a[0:limit], width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b[0:limit], width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("comparison of clat_percentiles<=95% bs=" + str(bs) + 'k')
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label[0:limit])
        subplot.set_xlabel('X%')
        subplot.set_ylabel("clat_percentiles")
        plt.grid(True)
        # plt.show()
        plt.savefig('images' + os.sep + io_pattern + os.sep + 'clat_percentilesA' + str(bs) + '.png')
        plt.close()

    def get_single_rwcomparison_clatB(self, io_pattern='read', bs=1, limit=13):
        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_a_var = None
        for ele in parameter_a_list:
            if (ele.test_parameter['bs'] == bs):
                parameter_a_var = ele
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_b_var = None
        for ele in parameter_b_list:
            if (ele.test_parameter['bs'] == bs):
                parameter_b_var = ele

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

        parameter_a_clat = sorted(parameter_a_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])
        parameter_b_clat = sorted(parameter_b_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])

        # parameter_a
        for v in parameter_a_clat:
            y_coordinate_parameter_a.append(float(v[1]))
            x_label.append(v[0])

        # parameter_b_var
        for v in parameter_b_clat:
            y_coordinate_parameter_b.append(float(v[1]))

        x_coordinate = np.arange(1, len(parameter_a_clat) - limit + 1, 1)
        plt.figure(1, figsize=(9, 6))
        subplot = plt.subplot(1, 1, 1)
        width = 0.3

        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a[limit:], width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b[limit:], width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("comparison of clat_percentiles>95% bs=" + str(bs) + 'k')
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label[limit:])
        subplot.set_xlabel('X%')
        subplot.set_ylabel("clat_percentiles")
        plt.grid(True)
        # plt.show()
        plt.savefig('images' + os.sep + io_pattern + os.sep + 'clat_percentilesB' + str(bs) + '.png')
        plt.close()

    # lower  95%
    def get_rwcomparison_clatA(self, io_pattern='readwrite', bs=1, limit=12):
        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_a_var = None
        for ele in parameter_a_list:
            if (int(ele.test_parameter['bs']) == int(bs)):
                parameter_a_var = ele
                break
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_b_var = None
        for ele in parameter_b_list:
            if (int(ele.test_parameter['bs']) == int(bs)):
                parameter_b_var = ele
                break

        x_coordinate = np.arange(1, 1 + limit, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []
        fio_document = 'fio_read_document'
        parameter_a_clat = sorted(parameter_a_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])
        parameter_b_clat = sorted(parameter_b_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])
        # parameter_a
        for v in parameter_a_clat:
            y_coordinate_parameter_a.append(float(v[1]))
            x_label.append(v[0])

        # parameter_b_var
        for v in parameter_b_clat:
            y_coordinate_parameter_b.append(float(v[1]))

        plt.figure(1, figsize=(10, 8))
        subplot = plt.subplot(2, 1, 1)
        width = 0.3
        bar_parameter_a = subplot.bar(x_coordinate - width, y_coordinate_parameter_a[0:limit], width, color="green")
        bar_parameter_b = subplot.bar(x_coordinate, y_coordinate_parameter_b[0:limit], width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("comparison of clat_percentiles<=95% bs=" + str(bs) + 'k (read)')
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label[0:limit])
        subplot.set_xlabel('X%')
        subplot.set_ylabel("clat_percentiles")
        subplot.grid(True)
        '''
        readwrite-write
        '''
        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []
        fio_document = 'fio_write_document'
        parameter_a_clat = sorted(parameter_a_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])
        parameter_b_clat = sorted(parameter_b_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])
        # parameter_a
        for v in parameter_a_clat:
            y_coordinate_parameter_a.append(float(v[1]))
            x_label.append(v[0])

        # parameter_b_var
        for v in parameter_b_clat:
            y_coordinate_parameter_b.append(float(v[1]))

        subplot = plt.subplot(2, 1, 2)
        width = 0.3
        bar_parameter_a = subplot.bar(x_coordinate - width, y_coordinate_parameter_a[0:limit], width, color="green")
        bar_parameter_b = subplot.bar(x_coordinate, y_coordinate_parameter_b[0:limit], width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("comparison of clat_percentiles<=95% bs=" + str(bs) + 'k (write)')
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label[0:limit])
        subplot.set_xlabel('X%')
        subplot.set_ylabel("clat_percentiles")
        subplot.grid(True)
        # plt.show()
        plt.savefig('images' + os.sep + io_pattern + os.sep + 'clat_rwA' + str(bs) + '.png')
        plt.close()

    def get_rwcomparison_clatB(self, io_pattern='read', bs=1, limit=13):
        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_a_var = None
        for ele in parameter_a_list:
            if (ele.test_parameter['bs'] == bs):
                parameter_a_var = ele
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_b_var = None
        for ele in parameter_b_list:
            if (ele.test_parameter['bs'] == bs):
                parameter_b_var = ele

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'

        parameter_a_clat = sorted(parameter_a_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])
        parameter_b_clat = sorted(parameter_b_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])

        # parameter_a
        for v in parameter_a_clat:
            y_coordinate_parameter_a.append(float(v[1]))
            x_label.append(v[0])

        # parameter_b_var
        for v in parameter_b_clat:
            y_coordinate_parameter_b.append(float(v[1]))

        x_coordinate = np.arange(1, len(parameter_a_clat) - limit + 1, 1)
        plt.figure(1, figsize=(10, 8))
        subplot = plt.subplot(2, 1, 1)
        width = 0.3

        bar_parameter_a = subplot.bar(x_coordinate - width, y_coordinate_parameter_a[limit:], width, color="green")
        bar_parameter_b = subplot.bar(x_coordinate, y_coordinate_parameter_b[limit:], width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("comparison of clat_percentiles>95% bs=" + str(bs) + 'k (read)')
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label[limit:])
        subplot.set_xlabel('X%')
        subplot.set_ylabel("clat_percentiles")
        plt.grid(True)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_write_document'
        parameter_a_clat = sorted(parameter_a_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])
        parameter_b_clat = sorted(parameter_b_var.getattribute(fio_document).clat_percentiles.items(),
                                  key=lambda d: d[0])

        # parameter_a
        for v in parameter_a_clat:
            y_coordinate_parameter_a.append(float(v[1]))
            x_label.append(v[0])

        # parameter_b_var
        for v in parameter_b_clat:
            y_coordinate_parameter_b.append(float(v[1]))

        x_coordinate = np.arange(1, len(parameter_a_clat) - limit + 1, 1)
        subplot = plt.subplot(2, 1, 2)
        width = 0.3

        bar_parameter_a = subplot.bar(x_coordinate - width, y_coordinate_parameter_a[limit:], width, color="green")
        bar_parameter_b = subplot.bar(x_coordinate, y_coordinate_parameter_b[limit:], width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("comparison of clat_percentiles>95% bs=" + str(bs) + 'k (write)')
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label[limit:])
        subplot.set_xlabel('X%')
        subplot.set_ylabel("clat_percentiles")
        subplot.grid(True)
        # plt.show()
        plt.savefig('images' + os.sep + io_pattern + os.sep + 'clat_rwB' + str(bs) + '.png')
        plt.close()

    def get_single_rwcomparison_aggregate(self, io_pattern, property_name, y_label='aggregate bandwidth (KB/S)'):
        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

            # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.getattribute(fio_document).bw[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.getattribute(fio_document).bw[property_name]))

        plt.figure(1, figsize=(9, 6))
        subplot = plt.subplot(1, 1, 1)
        width = 0.3
        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("comparison of aggregate bandwidth" + str(property_name))
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel(y_label)
        plt.grid(True)
        plt.savefig('images' + os.sep + io_pattern + os.sep + 'aggrebw' + property_name + '.png')
        plt.close()

    def get_single_rwcomparison_cpu(self, io_pattern, property_name, y_label='CPU usage statistics'):
        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_a_list.sort(key=lambda x: x.test_parameter['bs'])
        parameter_b_list.sort(key=lambda x: x.test_parameter['bs'])
        x_coordinate = np.arange(1, len(parameter_a_list) + 1, 1)

        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

            # parameter_a
        for fio_rwdocument in parameter_a_list:
            y_coordinate_parameter_a.append(float(fio_rwdocument.cpu[property_name]))
            x_label.append(str(fio_rwdocument.test_parameter['bs']) + 'k')

        # parameter_b_var
        y_coordinate = []
        for fio_rwdocument in parameter_b_list:
            y_coordinate_parameter_b.append(float(fio_rwdocument.cpu[property_name]))

        plt.figure(1, figsize=(9, 6))
        subplot = plt.subplot(1, 1, 1)
        width = 0.3
        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b),
                       loc='upper right')
        subplot.set_title("CPU usage statistics " + str(property_name))
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('block size (K)')
        subplot.set_ylabel(y_label)
        plt.grid(True)
        plt.savefig('images' + os.sep + io_pattern + os.sep + 'cpu' + property_name + '.png')
        plt.close()

    def get_all_rwcomparison_clatA(self, io_pattern='read', bs=1, ylabel='Distribution of I/O clat(us)'):
        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_a_var = None
        for ele in parameter_a_list:
            if (int(ele.test_parameter['bs']) == int(bs)):
                parameter_a_var = ele
                break
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_b_var = None
        for ele in parameter_b_list:
            if (int(ele.test_parameter['bs']) == int(bs)):
                parameter_b_var = ele
                break
        x_coordinate = np.arange(1, 1 + len(parameter_b_var.lat_usec), 1)
        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

        parameter_a_clat = sorted(parameter_a_var.lat_usec.items(), key=lambda d: d[0])
        parameter_b_clat = sorted(parameter_b_var.lat_usec.items(), key=lambda d: d[0])

        tmp_x_label = []
        tmp_x_label.append(0)
        for v in parameter_a_clat:
            y_coordinate_parameter_a.append(float(v[1]))
            tmp_x_label.append(v[0])

        for index in range(len(tmp_x_label) - 2):
            x_label.append('(%d-%d]' % (tmp_x_label[index], tmp_x_label[index + 1]))
        x_label.append('(1,2]ms')

        # parameter_b_var
        for v in parameter_b_clat:
            y_coordinate_parameter_b.append(float(v[1]))

        plt.figure(1, figsize=(13, 5))
        subplot = plt.subplot(1, 1, 1)
        width = 0.3
        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("Distribution of I/O completion latencies <=1000 us bs=" + str(bs) + 'k')
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('latencies (us)')
        subplot.set_ylabel(ylabel)
        plt.grid(True)
        # plt.show()
        plt.savefig('images' + os.sep + io_pattern + os.sep + 'rwclatus' + str(bs) + '.png')
        plt.close()

    def get_all_rwcomparison_clatB(self, io_pattern='read', bs=1, ylabel='Distribution of I/O clat(ms)'):
        # At first ,we should check whether property_name is legal
        parameter_a_list = self.documents[io_pattern][self.parameter_a]
        parameter_a_var = None
        for ele in parameter_a_list:
            if (int(ele.test_parameter['bs']) == int(bs)):
                parameter_a_var = ele
                break
        parameter_b_list = self.documents[io_pattern][self.parameter_b]
        parameter_b_var = None
        for ele in parameter_b_list:
            if (int(ele.test_parameter['bs']) == int(bs)):
                parameter_b_var = ele
                break
        x_coordinate = np.arange(1, len(parameter_b_var.lat_msec) + 1, 1)
        y_coordinate_parameter_a = []
        y_coordinate_parameter_b = []
        x_label = []

        fio_document = 'fio_read_document'
        if ('write' in io_pattern):
            fio_document = 'fio_write_document'

        parameter_a_clat = sorted(parameter_a_var.lat_msec.items(), key=lambda d: d[0])
        parameter_b_clat = sorted(parameter_b_var.lat_msec.items(), key=lambda d: d[0])

        # parameter_a
        tmp_x_label = []
        tmp_x_label.append(0)
        for v in parameter_a_clat:
            y_coordinate_parameter_a.append(float(v[1]))
            tmp_x_label.append(v[0])

        for index in range(len(tmp_x_label) - 2):
            if (tmp_x_label[index] < 100):
                x_label.append('(%d,%d]' % (tmp_x_label[index], tmp_x_label[index + 1]))
            elif (tmp_x_label[index] >= 100 and tmp_x_label[index] < 1000):
                a = round(tmp_x_label[index] / 1000, 2)
                b = round(tmp_x_label[index + 1] / 1000, 2)
                x_label.append('(%.1f,%.1f]' % (a, b))
            else:
                a = tmp_x_label[index] / 1000
                b = tmp_x_label[index + 1] / 1000
                x_label.append('(%d,%d]' % (a, b))
        x_label.append('>=2s')
        # parameter_b_var
        for v in parameter_b_clat:
            y_coordinate_parameter_b.append(float(v[1]))

        plt.figure(1, figsize=(13, 6))
        subplot = plt.subplot(1, 1, 1)
        width = 0.42
        bar_parameter_a = plt.bar(x_coordinate - width, y_coordinate_parameter_a, width, color="green")
        bar_parameter_b = plt.bar(x_coordinate, y_coordinate_parameter_b, width, color="red")
        subplot.legend((bar_parameter_a[0], bar_parameter_b[0]), (self.parameter_a, self.parameter_b), loc='upper left')
        subplot.set_title("Distribution of I/O completion latencies > 1ms bs=" + str(bs) + 'k')
        subplot.set_xticks(x_coordinate)
        subplot.set_xticklabels(x_label)
        subplot.set_xlabel('latencies (ms/s)')
        subplot.set_ylabel(ylabel)
        plt.grid(True)
        # plt.show()
        plt.savefig('images' + os.sep + io_pattern + os.sep + 'rwclatms' + str(bs) + '.png')
        plt.close()
