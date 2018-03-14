#!/bin/bash
# POSIX
#
#description:    install capes
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2018-01-22
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

print_message "MULTEXU_INFO" "Now start to install capes..."
export PYTHONPATH=$PYTHONPATH:/usr/local/lib/python3.5/site-packages/

print_message "MULTEXU_INFO" "1.upgrade python..."
print_message "MULTEXU_INFO" "2.install sqlite..."
`${PAUSE_CMD}`
`${PAUSE_CMD}`

print_message "MULTEXU_INFO" "Now start to install capes..."
#检测和节点的状态：是否可达  ssh端口22是否启用
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu_ssh.sh  --test_host_available=nodes_all.out
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu_ssh.sh  --test_host_ssh_enabled=nodes_all.out
wait_util_cluster_host_available "nodes_all.out" ${sleeptime} ${limit}
wait_util_cluster_ssh_enabled "nodes_all.out" ${sleeptime} ${limit}
`${PAUSE_CMD}`

print_message "MULTEXU_INFO" "Now start to upgrade python and pip..."
#clear_execute_statu_signal ${AIOCC_EXECUTE_SIGNAL_FILE} 由具体被调用的脚本执行信号量清除操作
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="sh ${AIOCC_BATCH_DIR}/_update_to_python3.5.2.sh"
ssh_check_cluster_status nodes_all.out "${AIOCC_EXECUTE_STATUS_FINISHED}" ${sleeptime} ${limit} "${AIOCC_EXECUTE_SIGNAL_FILE}"
print_message "MULTEXU_INFO" "Finished to upgrade python and pip..."

print_message "MULTEXU_INFO" "Now start to install capes dependencies..."
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="sh ${AIOCC_BATCH_DIR}/__capes_install.sh"
ssh_check_cluster_status nodes_all.out "${AIOCC_EXECUTE_STATUS_FINISHED}" ${sleeptime} ${limit} "${AIOCC_EXECUTE_SIGNAL_FILE}"
print_message "MULTEXU_INFO" "Finished to install capes dependencies..."

print_message "MULTEXU_INFO" "Finished to install capes..."
