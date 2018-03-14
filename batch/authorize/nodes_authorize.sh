# vim ClientAuthorize.sh
#!/bin/bash
#
#description:   a simple uniform tools for authorization
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2016-07-20
#

#声明环境变量
export PATH="/usr/lib/qt-3.3/bin:/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin"
export LANG="en_US.UTF-8"

#检查所需目录及文件,如果没有就创建一个
if [ ! -d /root/.ssh ];then
    mkdir /root/.ssh
fi

if [ ! -f /root/.ssh/authorized_keys ];then
    touch /root/.ssh/authorizedzz_keys
fi

setenforce 0
sed -i 's/SELINUX=enforcing/SELINUX=disabled/g' /etc/selinux/config;
echo "MULTEXU INFO:set SELINUX=disabled"

#设置被管理机的相关目录文件权限
chmod go-w /root
chmod 700 /root/.ssh
chmod 600 /root/.ssh/*
 
#set authorized_keys
echo "ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDCH9QFfsI9MXLlIOGTPjoDU/rsnSl+iVdvx3wWqMswl22hIQEJzicTZ4ujgVgepN8asbDQlte2p+CUL7WBQXFj5CpOe6TwidEZpVRWC2C66Ylvd4NBkWllyurJMc0Hpw1qbWEcZi56j3XGFY1kYy+M5NwxYtLz6uDr7KbGl1JD+Q15FpQXTg1fqEdkf2zsjXvfBW5melw9E+iCqQ14KMnLuNQLEqK+Ko86L8J1T40dKZK0WCgCDhpGR3Qxpd2IgKun/fv0g6X23w0mC9UBolmCMDqapg+vL/w/DrJF0mEhQgGe9hcoRB74D0e8QxFMtdhUWKooSPj6HlshrVVdKDkd root@localhost.localdomain" >> /root/.ssh/authorized_keys
