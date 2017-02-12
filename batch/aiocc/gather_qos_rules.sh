#!/bin/sh
#POSIX
#

#description:    gather qos rules from client nodes 
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-04
#
#运行本脚本时,假设主控制节点与所有节点(包括编译节点)已经进行SSH认证
#

#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ./__aiocc_init.sh ]; then
	echo "AIOCC Error:initialization failure:cannot find the file __aiocc_init.sh... "
	exit 1
else
	source ./__aiocc_init.sh
fi

source "${MULTEXU_BATCH_CRTL_DIR}/multexu_lib.sh"
clear_execute_statu_signal "${AIOCC_EXECUTE_SIGNAL_FILE}"

candidate_test_dir=$1
quiet=$2
#
#client节点收集本节点的qos_rules到${candidate_test_dir}
#
print_message "MULTEXU_INFO" "gathering qos rules from nodes_client.out..."
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_client.out --cmd="sh ${AIOCC_BATCH_DIR}/__gather_qos_rules.sh ${candidate_test_dir}"
ssh_check_cluster_status "nodes_client.out" "${AIOCC_EXECUTE_STATUS_FINISHED}" "1" "1" "${AIOCC_EXECUTE_SIGNAL_FILE}"
rm -f ${candidate_test_dir}/*.rule
auto_mkdir ${candidate_test_dir} "force"

#
#复制各节点${candidate_test_dir}目录下的qos_rules文件到当前节点的${candidate_test_dir}目录下
#
if [ ${quiet} -eq 1 ];then
    for host_ip in $(cat "${MULTEXU_BATCH_CONFIG_DIR}/nodes_client.out")
    do 
        scp root@${host_ip}:${candidate_test_dir}/*.rule ${candidate_test_dir}/ >/dev/null
    done
else
   for host_ip in $(cat "${MULTEXU_BATCH_CONFIG_DIR}/nodes_client.out")
   do 
        scp root@${host_ip}:${candidate_test_dir}/*.rule ${candidate_test_dir}/
   done
fi

send_execute_statu_signal "${AIOCC_EXECUTE_STATUS_FINISHED}" "${AIOCC_EXECUTE_SIGNAL_FILE}"

