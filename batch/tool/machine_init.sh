#!/bin/bash
# POSIX
#
#description:    新安装的机器安装一些软件
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2018-01-19
#
#initialization
cd "$( dirname "${BASH_SOURCE[0]}" )" #get  a Bash script tell what directory it's stored in
if [ ! -f ../ctrl/__init.sh ]; then
        echo "MULTEXU Error:multexu initialization failure:cannot find the file __init.sh... "
    exit 1
else
    source ../ctrl/__init.sh
fi

yum -y install lrzsz
yum -y install gcc
yum -y groupinstall "Development Libraries"
yum -y groupinstall "Development Tools"
yum -y install ncurses-devel zlib-devel texinfo gtk+-devel gtk2-devel qt-devel tcl-devel tk-devel libX11-devel kernel-headers kernel-devel
yum -y install vim
yum -y install ifconfig
yum -y install net-tools
yum -y install rpcbind
rpm -ql rpcbind
service rpcbind start
yum -y install nfs-utils
rpm -ql nfs-utils
service nfs start
mkdir /mnt/share
mount -t nfs 192.168.3.181:/home/development/ /mnt/share
