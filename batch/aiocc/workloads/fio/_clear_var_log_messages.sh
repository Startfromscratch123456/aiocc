#!/bin/bash
# POSIX
#
#description:    clear logs
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:   2017-01-09
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
VAR_LOG_DIR="/var/log"
if [ ! -f ${AIOCC_BATCH_DIR}/${relative_path}/_clear_var_log_messages.cfg ];then
	echo "false" > ${AIOCC_BATCH_DIR}/${relative_path}/_clear_var_log_messages.cfg
fi

START_SIGNAL="cat ${AIOCC_BATCH_DIR}/${relative_path}/_clear_var_log_messages.cfg"
if [ x`$START_SIGNAL` = x"true" ];then
	print_message "MULTEXU_INFO" "_clear_var_log_messages.sh is runing,no need to run it again..."
	exit 0
fi
echo "true" > ${AIOCC_BATCH_DIR}/${relative_path}/_clear_var_log_messages.cfg
while [ x`$START_SIGNAL` = x"true" ]
do
    :>${VAR_LOG_DIR}/messages
    sleep 3600s
done

