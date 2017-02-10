# description:    calculate the stddevvar and mean value of bandwidth
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-06
#
import numpy as np
import sys

# input_file = "data"
# output_file = "out"
input_file = sys.argv[1]
output_file = sys.argv[2]
bandwidth_stddev=0
bandwidth_mean=0
with open(input_file, 'r') as bandwidth_record_reader:
    bandwidth_record_line = bandwidth_record_reader.readline()
    bandwidth_record = np.array(bandwidth_record_line.split(" ")).astype(np.float)
    bandwidth_stddev = np.std(bandwidth_record)
    bandwidth_mean = np.mean(bandwidth_record)
with open(output_file, "w") as bandwidth_stat_writer:
    print("bandwidth_stddev:%d\nbandwidth_mean:%d" % (int(bandwidth_stddev),int(bandwidth_mean)),file=bandwidth_stat_writer)
