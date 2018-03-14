#!/bin/bash
# POSIX
#
#description:    install lustre client
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2018-01-20

#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../ctrl/__init.sh ]; then
    echo "MULTEXU Error:multexu initialization failure:cannot find the file __init.sh... "
    exit 1
else
    source ../ctrl/__init.sh
fi

source "${MULTEXU_BATCH_CRTL_DIR}"/multexu_lib.sh #调入multexu库
clear_execute_statu_signal   
                                                              
print_message "MULTEXU_INFO" "install dependencies..."                                       
cd ${MULTEXU_SOURCE_INSTALL_DIR}
print_message "MULTEXU_INFO" "enter directory ${MULTEXU_SOURCE_INSTALL_DIR}..."

rpm -ivh kmod-lustre-client-2.9.0*.rpm --nodeps --force
wait
rpm -ivh lustre-client-2.9.0*.rpm --force --nodeps
wait
rpm -ivh kmod-lustre-2.9.0*.rpm --force --nodeps
wait

send_execute_statu_signal "${MULTEXU_STATUS_EXECUTE}"
print_message "MULTEXU_INFO" "leave directory $( dirname "${BASH_SOURCE[0]}" )..."
print_message "MULTEXU_INFO" "all jobs finished"
#加载模块
modprobe lustre
exit 0
