#!/usr/bin/python
# -*- coding: UTF-8 -*-

from  FioDocument import *
from FileUtils import *


class AvgFioDocument:
    def __init__(self):
        self.avg_fio_document = FioRWDocument()
        self.avg_fio_document.fio_read_document = FioDocument()
        self.avg_fio_document.fio_write_document = FioDocument()
        self.count = 0;

    def add_fio_document(self, fio_document):
        self.count += 1;
        for property_name, value in vars(fio_document).items():
            if (isinstance(value, dict) and property_name != "test_parameter"):
                for k, v in value.items():
                    self.avg_fio_document.__getattribute__(property_name)[k] += v
        if not fio_document.fio_read_document is None:
            for property_name, value in vars(fio_document.fio_read_document).items():
                if (isinstance(value, dict)):
                    for k, v in value.items():
                        self.avg_fio_document.fio_read_document.__getattribute__(property_name)[k] += v

        if not fio_document.fio_write_document is None:
            for property_name, value in vars(fio_document.fio_write_document).items():
                if (isinstance(value, dict)):
                    for k, v in value.items():
                        self.avg_fio_document.fio_write_document.__getattribute__(property_name)[k] += v

        #for k, v in fio_document.getattribute("test_parameter").items():
            #self.avg_fio_document.__getattribute__("test_parameter")[k] = v

    def get_avg_fio_document(self):
        for property_name, value in vars(self.avg_fio_document).items():
            if (isinstance(value, dict) and property_name != "test_parameter"):
                for k, v in value.items():
                    value[k] /= self.count
        if  self.avg_fio_document.fio_read_document is not None:
            for property_name, value in vars(self.avg_fio_document.fio_read_document).items():
                if (isinstance(value, dict)):
                    for k, v in value.items():
                        value[k] /= self.count

        if  self.avg_fio_document.fio_write_document is not None:
            for property_name, value in vars(self.avg_fio_document.fio_write_document).items():
                if (isinstance(value, dict)):
                    for k, v in value.items():
                        value[k] /= self.count
        return self.avg_fio_document



