# -*- coding: utf-8 -*-

#文件内读写合一起
file_name1 = "data1"
rs = ""
with open(file_name1, "r+") as record_file:
    first = record_file.readline().strip('\n')
    second = record_file.readline().strip('\n')
    rs = ""
    while first:
        rs += str(float(first) + float(second)) + "\n"
        first = record_file.readline().strip('\n')
        if not first:
            break
        second = record_file.readline().strip('\n')
file_name2 = "data2"
with open(file_name2, "w+") as record_file2:
    record_file2.writelines(rs)

#两个客户端求平均
rs = ""
with open(file_name2, "r+") as record_file:
    first = record_file.readline().strip('\n')
    second = record_file.readline().strip('\n')
    rs = ""
    while first:
        rs += str((float(first) + float(second))/2) + "\n"
        first = record_file.readline().strip('\n')
        if not first:
            break
        second = record_file.readline().strip('\n')
file_name3 = "data3"
with open(file_name3, "w+") as record_file:
    record_file.writelines(rs)

#三个样本求平均
with open(file_name3, "r+") as record_file:
    first = record_file.readline().strip('\n')
    second = record_file.readline().strip('\n')
    third = record_file.readline().strip('\n')
    rs = ""
    while first:
        rs += str((float(first) + float(second) + float(third)) / 3) + "\n"
        first = record_file.readline().strip('\n')
        if not first:
            break
        second = record_file.readline().strip('\n')
        third = record_file.readline().strip('\n')

file_name4 = "data4"
with open(file_name4, "w+") as record_file:
    record_file.writelines(rs)
