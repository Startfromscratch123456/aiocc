#!/bin/bash
# POSIX
#
#description:    install capes
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


print_message "MULTEXU_INFO" "Now start to install capes..."
cd ${MULTEXU_SOURCE_TOOL_DIR}
print_message "MULTEXU_INFO" "Entering directory ${MULTEXU_SOURCE_TOOL_DIR}..."
#install setuptools-32
export PYTHONPATH=$PYTHONPATH:/usr/local/lib/python3.5/site-packages/

pip install numpy
pip install pysqlite
pip install zmq
pip install python_daemon-2.1.2-py2.py3-none-any.whl
`${PAUSE_CMD}`

send_execute_statu_signal "${AIOCC_EXECUTE_STATUS_FINISHED}" "${AIOCC_EXECUTE_SIGNAL_FILE}"
print_message "MULTEXU_INFO" "Finished to install capes..."
`${PAUSE_CMD}`
exit 0
