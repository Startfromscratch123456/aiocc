#!/bin/bash
# POSIX
#
#description:    build lustre 2.8.0 automaticlly [lustre client]
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2018-01-19
#

#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../ctrl/__init.sh ]; then
    echo "MULTEXU Error:initialization failure:cannot find the file __init.sh... "
    exit 1
else
    source ../ctrl/__init.sh
    echo 'MULTEXU INFO:initialization completed...'
    `${PAUSE_CMD}`
fi

source "${MULTEXU_BATCH_CRTL_DIR}/multexu_lib.sh"
clear_execute_statu_signal

sleeptime=60 #设置检测的睡眠时间
limit=10 #递减下限
#计算程序运行的时间
start_time=$(date +%s%N)
start_time_ms=${start_time:0:16}

#
#if you login in the system as root, after this command,then you will enter /root directory
#
cd $HOME
BUILD_BASE_DIR="$HOME""/kernel/rpmbuild"

#grep -Ri 'intel' /usr
#rpm -ivh $PKG_PATH/kernel-*.rpm
#/sbin/new-kernel-pkg --package kernel --mkinitrd --dracut --depmod --install 2.6.32.431.5.1.el6_lustre
#reboot
#多次重复编译时，不需要每次都安装依赖，可以指定此选项为1，跳过安装依赖以节约时间
skip_install_dependency=0
while :;
do
    case $1 in
        --skip_install_dependency=?*)
            skip_install_dependency=${1#*=}
            shift
            ;;
        -?*)
            printf 'WARN: Unknown option (ignored): %s\n' "(" >&2")"
            shift
            ;;
        *)    # Default case: If no more options then break out of the loop.
            shift
            break
    esac        
done

if [ ${skip_install_dependency} -eq 0 ];then 
    print_message "MULTEXU_INFO" "install dependencies..."  
    #
    #             yum -y install quilt
    #
    yum --nogpgcheck localinstall ${MULTEXU_SOURCE_BUILD_DIR}/newt-devel-0.52.15-4.el7.x86_64.rpm ${MULTEXU_SOURCE_BUILD_DIR}/slang-devel-2.2.4-11.el7.x86_64.rpm  ${MULTEXU_SOURCE_BUILD_DIR}/asciidoc-8.6.8-5.el7.noarch.rpm 
    sleep ${sleeptime}s
    yum -y groupinstall "Development Tools"
    sleep ${sleeptime}s
    yum -y install xmlto 
    `${PAUSE_CMD}`
    yum -y install asciidoc 
    `${PAUSE_CMD}`
    yum -y install elfutils-libelf-devel 
    `${PAUSE_CMD}`
    yum -y install zlib-devel 
    `${PAUSE_CMD}`
    yum -y install binutils-devel
    `${PAUSE_CMD}`
    yum -y install newt-devel 
    `${PAUSE_CMD}`
    yum -y install python-devel 
    `${PAUSE_CMD}`
    yum -y install hmaccalc 
    `${PAUSE_CMD}`
    yum -y install perl-ExtUtils-Embed  
    `${PAUSE_CMD}`
    yum -y install python-docutils 
    `${PAUSE_CMD}`
    yum -y install elfutils-devel 
    `${PAUSE_CMD}`
    yum -y install audit-libs-devel 
    `${PAUSE_CMD}`
    yum -y install libselinux-devel 
    `${PAUSE_CMD}`
    yum -y install ncurses-devel 
    `${PAUSE_CMD}`
    yum -y install pesign 
    `${PAUSE_CMD}`
    yum -y install linux-firmware
    `${PAUSE_CMD}`
    yum -y install numactl-devel 
    `${PAUSE_CMD}`
    yum -y install bc
    `${PAUSE_CMD}`
    yum -y install pciutils-devel 
    `${PAUSE_CMD}`
    yum -y install linux-firmware
    `${PAUSE_CMD}`
    yum -y install xfsprogs
    `${PAUSE_CMD}`
    yum -y install kmod 
    `${PAUSE_CMD}`
    rpm -ivh ${MULTEXU_SOURCE_BUILD_DIR}/epel-release-7-8.noarch.rpm 
fi

echo '%_topdir %(echo $HOME)/kernel/rpmbuild' > ~/.rpmmacros
rpm -ivh ${MULTEXU_SOURCE_BUILD_DIR}/lustre-client-2.8.0-3.10.0_327.3.1.el7.x86_64.src.rpm  2>&1 | grep -v exist
wait
print_message "MULTEXU_INFO" "Now start to rpmbuild  lustre(client)..."

cd "${BUILD_BASE_DIR}"
rpmbuild -bp --target=`uname -m` ./SPECS/lustre.spec
sleep ${sleeptime}s
wait

cd "${BUILD_BASE_DIR}"/BUILD/lustre-2.8.0/

print_message "MULTEXU_INFO" "now start to patch the lustre ..."
#patch -p1 < ${MULTEXU_SOURCE_BUILD_DIR}/lustre_nrs_sscdt.patch
#patch -p1 < ${MULTEXU_SOURCE_BUILD_DIR}/lustre_qos.patch
`${PAUSE_CMD}`
print_message "MULTEXU_INFO" "now start to run _patch_lustre.sh..."
sh ${MULTEXU_BATCH_BUILD_DIR}/_patch_lustre.sh
`${PAUSE_CMD}`

#注意--with-linux指定的位置
wait
./configure --with-linux="${BUILD_BASE_DIR}"/BUILD/kernel-3.10.0_lustre.x86_64/ --disable-server  
wait
make rpms -j8
print_message "MULTEXU_INFO" "finished to make rpms (client) ..."
`${PAUSE_CMD}`
send_execute_statu_signal "${MULTEXU_STATUS_EXECUTE}"

end_time=$(date +%s%N)
end_time_ms=${end_time:0:16}
#scale=6
time_cost=0
time_cost=`echo "scale=6;($end_time_ms - $start_time_ms)/1000000" | bc` 
print_message "MULTEXU_INFO" "Total time spent:${time_cost} s"

exit 0


