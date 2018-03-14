#!/usr/bin/python
# -*- coding: UTF-8 -*-
from FileUtils import *


def print_fio_document(fio_rwdocument):
    if not fio_rwdocument.fio_read_document is None:
        fio_rwdocument.fio_read_document.print_all_kv()
    if not fio_rwdocument.fio_write_document is None:
        fio_rwdocument.fio_write_document.print_all_kv()
    fio_rwdocument.print_all_kv()
