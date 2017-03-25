 #!/bin/sh
#POSIX
#

#description:    
#a,b,c均为大于0的自然数：
#a/(b+c) + b/(a+c) + c/(a+b) = 4
#1.编一个程序证明在a<65536,b<65536,c<65536情况下无解
#2.编一个程序证明以下值是正确的：
#a=4373612677928697257861252602371390152816537558161613618621437993378423467772036
#b=36875131794129999827197811565225474825492979968971970996283137471637224634055579
#c=154476802108746166441951315019919837485664325669565431700026634898253202035277999
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-03-25
#
#运行本脚本时,假设主控制节点与所有节点(包括编译节点)已经进行SSH认证
#
#
sleeptime=120 #设置检测的睡眠时间
limit=10 #递减下限

#
#计算程序运行的时间
#
start_time=$(date +%s%N)
start_time_ms=${start_time:0:16}

#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../ctrl/__init.sh ]; then
    echo "MULTEXU Error:multexu initialization failure:cannot find the file __init.sh... "
    exit 1
else
    source ../ctrl/__init.sh
fi
source "${MULTEXU_BATCH_CRTL_DIR}"/multexu_lib.sh #调入multexu库



function __clear()
{
    end_time=$(date +%s%N)
    end_time_ms=${end_time:0:16}
    #scale=6
    time_cost=0
    time_cost=`echo "scale=6;($end_time_ms - $start_time_ms)/1000000" | bc`
    print_message "MULTEXU_INFO" "computing process finished..."
    print_message "MULTEXU_INFO" "Total time spent:${time_cost} s"
}
function upper_round()
{
    local x
    local y
    local z
    local m
    local n
    local re
    let x=$1
    let y=$2
    let z=$x/$y
    let m=$x%$y
    let n=$y/2
    if [ $m -lt $n ]; then
       let re=$z
    else
       let re=$z+1
    fi
    eval "$3=$re"
}


#分发源文件到各个计算节点
iptable_file="nodes_compute.out"
range_limit=65536
tencent2017_interview_dir="${MULTEXU_BATCH_DIR}/tmp"
auto_mkdir "${tencent2017_interview_dir}/result" "force"

sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${iptable_file} --cmd="rm -f ${tencent2017_interview_dir}/tencent2017_interview*"
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${iptable_file} --sendfile="${tencent2017_interview_dir}/tencent2017_interview.c" --location="${tencent2017_interview_dir}/" 

sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${iptable_file} --cmd="rm -rf ${tencent2017_interview_dir}/result"
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${iptable_file} --cmd="mkdir ${tencent2017_interview_dir}/result"

`${PAUSE_CMD}`
#执行编译
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${iptable_file} --cmd="cd ${tencent2017_interview_dir}/ && gcc tencent2017_interview.c -o tencent2017_interview -lpthread"
#计算计算节点的个数,以分配计算任务
nodes_num=`wc -l ${MULTEXU_BATCH_CONFIG_DIR}/${iptable_file} | cut -d ' ' -f 1`
start_index=65530
low=${start_index}
high=1
distance=$(( ${range_limit}-${low} ))
step=1
upper_round ${distance} ${nodes_num} step
#step=`echo "( $range_limit - $low ) / $nodes_num" | bc`
index=0

ip_table_array=($(cat ${MULTEXU_BATCH_CONFIG_DIR}/${iptable_file}))
cd ${tencent2017_interview_dir}/ && gcc tencent2017_interview.c -o tencent2017_interview -lpthread
#验证给定的值是正确的
print_message "MULTEXU_INFO" "Q2.编一个程序证明以下值是正确的："
print_message "MULTEXU_ECHOX" "a=4373612677928697257861252602371390152816537558161613618621437993378423467772036"    
print_message "MULTEXU_ECHOX" "b=36875131794129999827197811565225474825492979968971970996283137471637224634055579"    
print_message "MULTEXU_ECHOX" "c=154476802108746166441951315019919837485664325669565431700026634898253202035277999"    
./tencent2017_interview Q2
print_message "MULTEXU_ECHOX" " "   
print_message "MULTEXU_ECHOX" " "  
echo ""
`${PAUSE_CMD}` 
print_message "MULTEXU_INFO" "Q1.编一个程序证明在a<65536,b<65536,c<65536情况下无解..."
#分发任务到各节点
while [ ${low} -le ${range_limit} ];
do 
    high=$(( ${low}+${step} ))
    #在某个节点上进行[low, high]范围内的最外层循环
    ssh -f "${ip_table_array[${index}]}" "cd ${tencent2017_interview_dir}/ && ./tencent2017_interview Q1 ${low} ${high}"
    print_message "MULTEXU_INFO" "${ip_table_array[${index}]} execute range[${low}, ${high}]..."    
    low=${high}
    (( index += 1 ))
done
signal_file="${tencent2017_interview_dir}/result/Q1*"
#检测任务完成状态,检测到任意一个节点输出的解的个数大于0,就停止检测
count=0
while [ true ];
do
    print_message "MULTEXU_INFO" "waiting nodes which its ip in ${iptable_file} computing,the next check time will be ${sleeptime}s later..."
    sleep ${sleeptime}s
    count=0

    for host_ip in ${ip_table_array[*]}
    do
        retval=`ssh -f ${host_ip} "cat ${signal_file}"`
        #retval=$?
        if [ "x$retval" != "x" ];then
            if [ $retval -gt 0 ];then
                print_message "MULTEXU_INFO" "node ${host_ip} find at least one solution..."
                #终止所有任务
                #send_execute_statu_signal "" ""
                __clear
                exit 0
            elif [ $retval -eq 0 ];then
                (( count += 1 ))
                #全部执行完毕 且没有找到解
                if [ ${count} -eq ${nodes_num} ];then 
                    break 2
                fi
            fi
        fi       
    done
    if [ $sleeptime -gt $limit ];then
        let sleeptime/=2
    fi
done
print_message "MULTEXU_INFO" "No solution in range [${start_index}, ${range_limit}] ..."
__clear

#sh /home/development/LustreTools/batch/ctrl/multexu.sh --iptable=nodes_compute.out --sendfile=/home/development/LustreTools --location=/home/development/









