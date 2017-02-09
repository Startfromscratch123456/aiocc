#!/bin/sh
#POSIX
#

#description:    client nodes extract bandwidth from ${LUSTRE_PROC_OSC} directory
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-05
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

LUSTRE_PROC_OSC="/proc/fs/lustre/osc"
#注意是数组
OSC_ARRAY=($(ls ${LUSTRE_PROC_OSC}))

rm -f $1/*.import
auto_mkdir $1 "force"

for OSC in ${OSC_ARRAY[*]}
do
	cp ${LUSTRE_PROC_OSC}/${OSC}/import $1/${HOSTNAME}_${OSC}.import
done
send_execute_statu_signal "${AIOCC_EXECUTE_STATUS_FINISHED}" "${AIOCC_EXECUTE_SIGNAL_FILE}"
