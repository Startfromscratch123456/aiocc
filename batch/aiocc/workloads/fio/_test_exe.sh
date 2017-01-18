#!/bin/bash
# POSIX
#
#description:    execute test 
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
source "${MULTEXU_BATCH_CRTL_DIR}"/multexu_lib.sh #调入multexu库
#清除本地标记
clear_execute_statu_signal
#执行测试
$1
#写入测试完完成标记
send_execute_statu_signal "${MULTEXU_STATUS_EXECUTE}"
