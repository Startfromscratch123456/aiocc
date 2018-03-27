 #!/bin/bash
# POSIX
#
#description:    collect cpu and memory
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2018-03-23
#
#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../ctrl/__init.sh ]; then
        echo "MULTEXU Error:multexu initialization failure:cannot find the file __init.sh... "
    exit 1
else
    source ../ctrl/__init.sh
fi

source "${MULTEXU_BATCH_CRTL_DIR}/multexu_lib.sh"
clear_execute_statu_signal

time_str=`date +"%Y%m%d%H%M%S"`
memory_record="${MULTEXU_SOURCE_TOOL_DIR}/`hostname`_memory_${time_str}.txt"
cpu_record="${MULTEXU_SOURCE_TOOL_DIR}/`hostname`_cpu_${time_str}.txt"
touch ${memory_record}
touch ${cpu_record}
sleep_time=30

work_loop=`cat ${MULTEXU_BATCH_TOOL_DIR}/work_loop.cfg`

while [ x"$work_loop" == x"true" ];
do
    line1=`free -m | awk 'NR==2{print $2 " " $3" " $4" " $5" " $6" " $7}'`
    line2=`free -m | awk 'NR==3{print $2 " " $3" " $4}'`
    cpu=`cat /proc/stat | awk 'NR==1{print $2 " " $3" " $4" " $5" " $6" " $7" " $8" " $9" " $10}'`
    echo "${line1} ${line2}" >> ${memory_record}
    echo "${cpu}">> ${cpu_record}
    sleep ${sleep_time}s
done
exit 0
