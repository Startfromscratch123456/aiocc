# -*- coding: utf-8 -*-

import random

file_name1 = "data1"
rs = ""
with open(file_name1, "r+") as record_file1:
    first = record_file1.readline().strip('\n')
    second = record_file1.readline().strip('\n')
    rs = ""
    while first:
        rs += str(float(first) + float(second)) + "\n"
        first = record_file1.readline().strip('\n')
        if not first:
            break
        second = record_file1.readline().strip('\n')

file_name2 = "data2"
with open(file_name2, "w+") as record_file2:
    record_file2.writelines(rs)

file_name3 = "data2"
with open(file_name2, "r+") as record_file3:
    first = record_file3.readline().strip('\n')
    second = record_file3.readline().strip('\n')
    rs = ""
    while first:
        rs += str((float(first) + float(second)) / 2) + "\n"
        first = record_file3.readline().strip('\n')
        if not first:
            break
        second = record_file3.readline().strip('\n')

file_name4 = "data3"
with open(file_name4, "w+") as record_file4:
    record_file4.writelines(rs)
