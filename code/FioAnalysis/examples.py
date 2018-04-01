#!/usr/bin/python
# -*- coding: UTF-8 -*-
import os
import re


def fio_filter(root_path):
    fio_file = open(root_path, "r+")
    line_str = fio_file.readline()
    fio_file_content = ""
    while line_str:
        if line_str[0] == '+' and line_str[1] != '+':
            fio_file_content += line_str[1:]
        elif line_str[0] == ' ' and line_str[1] == ' ':
            fio_file_content += line_str
        else:
            fio_file_content += "//" + line_str[1:]
        line_str = fio_file.readline()

    fio_file.close()
    fio_file = open(root_path, "w+")
    fio_file.write(fio_file_content)
    fio_file.close()

fio_filter("data.c")
