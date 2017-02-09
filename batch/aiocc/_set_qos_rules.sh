#!/bin/sh
#POSIX
#

#description:    set qos ruls on current node
#     author:    ShijunDeng
#      email:    dengshijun1992@gmail.com
#       time:    2017-01-09
#
#运行本脚本时,假设主控制节点与所有节点(包括编译节点)已经进行SSH认证
#

lustre_osc_dir=/proc/fs/lustre/osc
osc_names=($(ls ${lustre_osc_dir}))
for osc_name in ${osc_names[*]}
do
    lctl set_param osc.${osc_name}.qos_rules="`cat $1`"
done
#规则文件$1设置完毕后没有再被保留的必要
rm -f $1

