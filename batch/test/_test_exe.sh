#!/bin/bash
# POSIX
#
#description:    execute test 
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2016-07-27
#
#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../ctrl/__init.sh ]; then
    echo "MULTEXU Error:multexu initialization failure:cannot find the file __init.sh... "
    exit 1
else
    source ../ctrl/__init.sh
fi

source "${MULTEXU_BATCH_CRTL_DIR}"/multexu_lib.sh #调入multexu库

cmd_var=$1
parent_dir=$2

#避免多个测试进程读写同一个测试文件
#每个测试进程有一个唯一的测试目录
sub_dir=`date +%s%N | md5sum | head -c 10`
directory="${parent_dir}/${sub_dir}"
while [ -d "${directory}" ];
do
    sub_dir=`date +%s%N | md5sum | head -c 10`
    directory="${parent_dir}/${sub_dir}"
done

auto_mkdir " ${directory}" "force"

cmd_var="${cmd_var} -directory=${directory}"

#执行测试
${cmd_var}
#清除本地标记

clear_execute_statu_signal
#写入测试完完成标记
send_execute_statu_signal "${MULTEXU_STATUS_EXECUTE}"
