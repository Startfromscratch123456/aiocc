#!/bin/sh
#POSIX
#

#description:    clear the temporary file/output file of a aiocc process
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-02-09
#
#运行本脚本时,假设主控制节点与所有节点(包括编译节点)已经进行SSH认证
#
#清除AIOCC运行的结果文件/临时文件,运行本脚本之后再次运行aiocc_start.sh的效果等同于首次全新的运行aiocc_start.sh
#
#

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

function __clear()
{
    local cmd_var="sh ${AIOCC_BATCH_DIR}/_aiocc_clear.sh"
	`$cmd_var`
    sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_client.out --cmd="${cmd_var}"
}

__clear
#计算程序运行的时间
end_time=$(date +%s%N)
end_time_ms=${end_time:0:16}
#scale=6
time_cost=0
time_cost=`echo "scale=6;($end_time_ms - $start_time_ms)/1000000" | bc`
print_message "MULTEXU_INFO" "AIOCC clear process finished..."
print_message "MULTEXU_INFO" "Total time spent:${time_cost} s"