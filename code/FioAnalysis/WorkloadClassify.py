#!/usr/bin/python
# -*- coding: UTF-8 -*-

import os
import numpy as np
from FioStatistic import *
from FileUtils import *


class WorkloadClassify:
    def __init__(self):
        self.nrs_policy = "2"
        self.documents = {'read': [], 'randread': [], 'write': [], 'randwrite': [], 'readwrite': [], 'randrw': []}
        self.rw_index = {'read': 0, 'randread': 1, 'write': 2, 'randwrite': 3, 'readwrite': 4, 'randrw': 5}

    def __load_data_from_file(self, rootpath):
        for filename in get_all_fio_test_files(rootpath):
            io_pattern = filename[filename.rfind(os.sep) + 1:]
            io_pattern = io_pattern[:io_pattern.find("-")]
            self.documents[io_pattern].append(fio_file_to_document(filename))

    def load_data(self, rootpath):
        self.__load_data_from_file(rootpath)
        row_data = []
        count = 0
        for k, v in self.documents.items():
            for fio_rw_doc in v:
                count += 1
                row_data.append(float(self.rw_index[k]))
                row_data.append(fio_rw_doc.test_parameter["bs"])
                row_data.extend(fio_rw_doc.lat_usec.values())
                row_data.extend(fio_rw_doc.lat_msec.values())
                row_data.extend(fio_rw_doc.cpu.values())

                if fio_rw_doc.fio_read_document:
                    fio_read_document = fio_rw_doc.fio_read_document
                else:
                    fio_read_document = FioDocument()

                if fio_rw_doc.fio_write_document:
                    fio_write_document = fio_rw_doc.fio_write_document
                else:
                    fio_write_document = FioDocument()

                row_data.extend(fio_read_document.prefix.values())
                row_data.extend(fio_read_document.clat.values())
                row_data.extend(fio_read_document.clat_percentiles.values())
                row_data.extend(fio_read_document.bw.values())
                row_data.extend(fio_read_document.all_jobs.values())

                row_data.extend(fio_write_document.prefix.values())
                row_data.extend(fio_write_document.clat.values())
                row_data.extend(fio_write_document.clat_percentiles.values())
                row_data.extend(fio_write_document.bw.values())
                row_data.extend(fio_write_document.all_jobs.values())

        return np.array(row_data).reshape(count, -1)
