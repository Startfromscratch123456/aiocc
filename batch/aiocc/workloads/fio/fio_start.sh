#!/bin/bash
# POSIX
#
#description:   fio wordloads
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-09
#

#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../../__aiocc_init.sh ]; then
    echo "MULTEXU Error:multexu initialization failure:cannot find the file __i__aiocc_initnit.sh... "
    exit 1
else
    source ../../__aiocc_init.sh
fi

source "${MULTEXU_BATCH_CRTL_DIR}/multexu_lib.sh"  

sleeptime=20 #设置检测的睡眠时间
limit=10 #递减下限

relative_path="workloads/fio"
#测试结果存放目录
result_dir="${MULTEXU_TESTRESULT_DIR}/aiocc/fio"
EXIT_SIGNAL="cat ${AIOCC_BATCH_DIR}/${relative_path}/control.signal"
echo "RUN" > ${AIOCC_BATCH_DIR}/${relative_path}/control.signal
#测试参数
#test parameters
blocksize=1 #the blocksize
#测试目录
directory="/mnt/lustre/test"
direct=0
iodepth=5
allow_mounted_write=1
ioengine="sync"
special_cmd="-rwmixread=50" #随机IO时的一些特殊参数
size="512M"
numjobs=8
runtime=120
name="aiocc_fio_workloads"

blocksize_start=1
blocksize_end=1024
blocksize_multi_step=2
#设置检测测试是否结束的时间以及检测的下限
checktime_init=120
checktime_lower_limit=10
#IO方式
declare -a rw_array;#Type of I/O pattern. 

#fio的读写方式
rw_array=("randrw" "readwrite" "write" "randwrite" "read" "randread" "randrw" "readwrite" "write" "randwrite" "read" "randread" "randrw" "readwrite" "write" "randwrite" "read" "randread")
#获取客户端的ip地址,只需要其中一个即可,用作向服务器发命令,清除测试产生的文件
client_ip=`head -1 ${MULTEXU_BATCH_CONFIG_DIR}/nodes_client.out`

skip_install_fio=0

#获取参数值
function get_parameter()
{
    while :; 
    do
        case $1 in
            -f=?*|--fio_cmd=?*) #特殊附加命令
                special_cmd=${1#*=}
                shift
                ;;
            -f|--fio_cmd=) # Handle the case of an empty 
                printf 'MULTEXU ERROR: "-f|--fio_cmd" requires a non-empty option argument.\n' >&2
                exit 1
                ;;
            --skip_install_fio=?*) #特殊附加命令
                skip_install_fio=${1#*=}
                shift
                ;;
            --skip_install_fio=) # Handle the case of an empty 
                printf 'MULTEXU ERROR: "--skip_install_fio" requires a non-empty option argument.\n' >&2
                exit 1
                ;;
            -?*)
                printf 'MULTEXU WARN: Unknown option (ignored): %s\n' "$1" >&2
                shift
                ;;
            *)    # Default case: If no more options then break out of the loop.
                shift
                break
        esac
    done
}
get_parameter $@

sh ${MULTEXU_BATCH_CRTL_DIR}/multexu_ssh.sh  --test_host_available=nodes_all.out
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu_ssh.sh  --test_host_ssh_enabled=nodes_all.out
`${PAUSE_CMD}`
#清除信号量  避免干扰
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="sh ${MULTEXU_BATCH_CRTL_DIR}/multexu_ssh.sh  --clear_execute_statu_signal"
#安装fio
#
if [ ${skip_install_fio} -eq 0 ];then
    print_message "MULTEXU_INFO" "now start to check fio tool in client nodes..."
    sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_client.out --supercmd="sh ${AIOCC_BATCH_DIR}/${relative_path}/_fio_install.sh"
    ssh_check_cluster_status "nodes_client.out" "${MULTEXU_STATUS_EXECUTE}" $((sleeptime/4)) $((limit/2))
    print_message "MULTEXU_INFO" "finished fio checking..."
    #清除信号量  避免干扰
    sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_client.out --cmd="sh ${MULTEXU_BATCH_CRTL_DIR}/multexu_ssh.sh  --clear_execute_statu_signal"
fi
#
#删除oss上因为测试产生的文件和测试目录
#
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${client_ip} --cmd="rm -rf ${directory}"
#建立测试目录
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${client_ip} --cmd="mkdir ${directory}/"
#设置lustre的stripe
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${client_ip} --cmd="lfs setstripe -c -1 ${directory}"
print_message "MULTEXU_INFO" "all ost have been used..."

cd ${MULTEXU_BATCH_TEST_DIR}/
print_message "MULTEXU_INFO" "enter directory ${MULTEXU_BATCH_TEST_DIR}..."
auto_mkdir "${result_dir}" "force"

#定时清除服务器上的日志,因为测试的过程中会产生大量的日志,很可能会占用大量的日志空间或者影响服务器的性能
sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="sh ${AIOCC_BATCH_DIR}/${relative_path}/_clear_var_log_messages.sh"
print_message "MULTEXU_INFO" "the script clear_var_log_messages.sh is running in ipall.out set..."
#
#开始测试
#
print_message "MULTEXU_INFO" "now start the test processes..."

function _cleanup()
{
    sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_client.out --cmd="sh ${MULTEXU_BATCH_CRTL_DIR}/multexu_ssh.sh  --clear_execute_statu_signal"
    #清除测试产生的垃圾文件
    sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${client_ip} --cmd="rm -f ${directory}/*"    
    sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_all.out --cmd="echo false > ${AIOCC_BATCH_DIR}/${relative_path}/_clear_var_log_messages.cfg"
    `${PAUSE_CMD}`
    print_message "MULTEXU_INFO" "all test jobs has been finished..."
}

for rw_pattern in ${rw_array[*]}
do
    #测试结果的存放目录
    dirname="${result_dir}/${rw_pattern}"
    auto_mkdir "${dirname}" "weak"
    
    print_message "MULTEXU_ECHO" "    rw_array:${rw_pattern}"
    for ((blocksize=${blocksize_start} ;blocksize <= ${blocksize_end}; blocksize*=${blocksize_multi_step}))
    do
        if [ x`$EXIT_SIGNAL` = x"EXIT" ];then
            print_message "MULTEXU_INFO" "EXIT SIGNAL detected..."
            _cleanup
            exit 0
        fi
        print_message "MULTEXU_ECHO" "        start a test..."           
        special_cmd_io_choice=
        
        if [[ ${rw_pattern} == "readwrite" ]] || [[ ${rw_pattern} == "randrw" ]];then
            special_cmd_io_choice=${special_cmd}
        fi

        cmdvar="${MULTEXU_SOURCE_TOOL_DIR}/fio/fio -directory=${directory} -direct=${direct} -iodepth ${iodepth} -thread -rw=${rw_pattern} ${special_cmd_io_choice} -allow_mounted_write=${allow_mounted_write} -ioengine=${ioengine} -bs=${blocksize}k -size=${size} -numjobs=${numjobs} -runtime=${runtime} -group_reporting -name=${name} "
        print_message "MULTEXU_ECHO" "        test command:${cmdvar}"
        #删除测试文件
        sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=${client_ip} --cmd="rm -f ${directory}/*"        
        sleep ${sleeptime}s
        #测试结果文件名称,组成方式:读写模式-调度器-块大小-k.txt
        filename="${rw_pattern}-${blocksize}-k.txt"
        touch "${dirname}/${filename}"
        echo "${cmdvar}" > ${dirname}/${filename}
        #测试结果写入文件
        #sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_client.out --supercmd="sh ${AIOCC_BATCH_DIR}/${relative_path}/_test_exe.sh \"${cmdvar}\" " >> ${dirname}/${filename}
        sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_client.out --supercmd="sh ${AIOCC_BATCH_DIR}/${relative_path}/_test_exe.sh \"${cmdvar}\" " >/dev/null
        #检测测试是否完成
        ssh_check_cluster_status "nodes_client.out" "${MULTEXU_STATUS_EXECUTE}" ${checktime_init} ${checktime_lower_limit}
        #清除标记
        sh ${MULTEXU_BATCH_CRTL_DIR}/multexu.sh --iptable=nodes_client.out --cmd="sh ${MULTEXU_BATCH_CRTL_DIR}/multexu_ssh.sh  --clear_execute_statu_signal"            
        print_message "MULTEXU_ECHO" "        finish this test..."
        `${PAUSE_CMD}`
    done #blocksize
done #rw_pattern

_cleanup

exit 0
