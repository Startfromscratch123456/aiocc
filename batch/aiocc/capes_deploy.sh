#!/bin/bash
# POSIX
#
#description:    deploy capes
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2018-01-27
#initialization
sleeptime=60 #设置检测的睡眠时间
limit=10 #递减下限

#
#计算程序运行的时间
#
start_time=$(date +%s%N)
start_time_ms=${start_time:0:16}

#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ./__aiocc_init.sh ]; then
    echo "AIOCC Error:initialization failure:cannot find the file __aiocc_init.sh... "
    exit 1
else
    source ./__aiocc_init.sh
    echo 'AIOCC INFO:initialization completed...'
fi

source "${MULTEXU_BATCH_CRTL_DIR}/multexu_lib.sh"
clear_execute_statu_signal
clear_execute_statu_signal ${AIOCC_EXECUTE_SIGNAL_FILE}


print_message "MULTEXU_INFO" "Now start to deploy capes..."
cd ${MULTEXU_SOURCE_TOOL_DIR}
print_message "MULTEXU_INFO" "Entering directory ${MULTEXU_SOURCE_TOOL_DIR}..."

print_message "MULTEXU_INFO" "revise /etc/hosts ..."

#sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="yes | cp ${MULTEXU_BATCH_CONFIG_DIR}/hosts /etc/hosts"
#cat ${MULTEXU_BATCH_CONFIG_DIR}/hosts_table | while read each_line
#do
#	sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="echo \"${each_line}\">>/etc/hosts"
#done

`${PAUSE_CMD}`

print_message "MULTEXU_INFO" "intfdaemon_service.sh conf.py start  ..."
#sh ${AIOCC_SOURCE_DIR}/capes/capes-oss/intfdaemon_service.sh conf.py start

print_message "MULTEXU_INFO" "dqldaemon_service.sh conf.py start  ..."
#sh ${AIOCC_SOURCE_DIR}/capes/capes-oss/dqldaemon_service.sh conf.py start
`${PAUSE_CMD}`

print_message "MULTEXU_INFO" "ma_service.sh conf.py start  ..."
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="chmod a+x ${AIOCC_SOURCE_DIR}/capes/capes-oss/*.sh conf.py "
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="sh ${AIOCC_SOURCE_DIR}/capes/capes-oss/ma_service.sh conf.py start"
`${PAUSE_CMD}`

send_execute_statu_signal "${MULTEXU_STATUS_EXECUTE}"

print_message "MULTEXU_INFO" "Finished to deploy capes..."
