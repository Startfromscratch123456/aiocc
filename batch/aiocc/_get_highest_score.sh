#!/bin/sh
#POSIX
#

#description:    get highest score 
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-13
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

#local score_line=`sh ${AIOCC_BATCH_DIR}/_get_highest_score.sh $round_summary_file`
round_summary_file=$1
#max_bandwidth_given=$3
max_score=0
max_score_line=
#${CANDIDATE},${SCORE},${CANDIDATE_AVG_BW},${CANDIDATE_AVG_VAR},${CANDIDATE_TRY}
for score_line in $(cat ${round_summary_file})
do
	score=`echo ${score_line} | cut -d, -f 2`
	if [ `echo "${score} > ${max_score}" | bc -l` -eq 1 ];then
		max_score=${score} 
		max_score_line=${score_line}
	fi
done
echo "${max_score_line}"
#eval "$rs=${max_score_line}"
#send_execute_statu_signal "${AIOCC_EXECUTE_STATUS_FINISHED}" "${AIOCC_EXECUTE_SIGNAL_FILE}"
