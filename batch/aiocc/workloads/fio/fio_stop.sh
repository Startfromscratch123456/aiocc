#!/bin/bash
# POSIX
#
#description:    stop fio wordloads 
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-09
#

#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../../__aiocc_init.sh ]; then
    echo "MULTEXU Error:multexu initialization failure:cannot find the file __aiocc_init.sh... "
    exit 1
else
    source ../../__aiocc_init.sh
fi

source "${MULTEXU_BATCH_CRTL_DIR}/multexu_lib.sh"  
relative_path="workloads/fio"
#测试结果存放目录
echo "EXIT" > ${AIOCC_BATCH_DIR}/${relative_path}/control.signal
exit 0
