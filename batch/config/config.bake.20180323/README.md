##说明
- hosts：格式同/etc/hosts文件：网络IP地址 主机名或域名 主机名别名，其中包含的节点为Lustre文件系统所有的mds、oss、client节点，在配置lmt的时候需要用以覆盖各节点下的/etc/hosts文件
- hostsfile：包含的节点为Lustre文件系统所有的mds、oss、client节点的主机别名，在配置lmt的时候需要放到各节点的/etc/目录下
- hosts_table：hosts除去头两行的剩下信息，方便程序处理（批量更改各节点的主机名）使用
- nodes_all.out：参见设计文档
- nodes_authorize.out：参见设计文档
- nodes_client.out：参见设计文档
- nodes_oss.out：参见设计文档
- nodes_server.out：参见设计文档
- multexu.signal:信号量文件


---
##注意
- ip列表文件必须是一个ip一行，不要添加其他任何注释以及无关信息，也不要加多余的空行
