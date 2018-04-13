#!/usr/bin/python
# -*- coding: UTF-8 -*-
#
# description: 解析xml文件中的host信息
#      author: ShijunDeng
#       email: dengshijun1992@gmail.com
#        time: 2017-10-10
#


from xml.dom import minidom, Node
from entity import hostnode, host_set, is_oss_node, is_mds_node, is_client_node
from config import conf_tool
import os


#
# 实现etc_host.xml文件与host_set之间的转换
#
class etc_host_handler:
    def __init__(self, doc):
        self.doc = doc

    #
    # 将database.conf中定义的etc_hosts=etc_hosts.xml转换为host_set对象
    #
    def xml_to_hosts(self):
        host_set_var = host_set()
        nodes = self.doc.documentElement.getElementsByTagName("host")
        for child_node in nodes:
            if child_node.nodeType == Node.ELEMENT_NODE and child_node.tagName == "host":
                host_set_var.add(self.__node_to_host(child_node))
        return host_set_var

    #
    # 将host_set对象转换为database.conf中定义的etc_hosts=etc_hosts.xml
    #
    def hosts_to_xml(self, hosts_set):
        hosts_node = self.doc.createElement("hosts")
        self.doc.appendChild(hosts_node)
        for host_node in hosts_set.hosts.values():
            hosts_node.appendChild(self.__host_to_node(host_node))
        return self.doc

    #
    # 将一个xml host节点转换为host对象
    #
    def __node_to_host(self, parent_node):
        host_node_var = hostnode()
        for child_node in parent_node.childNodes:
            if child_node.nodeType == Node.ELEMENT_NODE:
                if child_node.tagName == "ip":
                    host_node_var.ip = child_node.firstChild.nodeValue
                elif child_node.tagName == "domain":
                    host_node_var.domain = child_node.firstChild.nodeValue
                elif child_node.tagName == "hostname":
                    host_node_var.hostname = child_node.firstChild.nodeValue
                elif child_node.tagName == "property":
                    host_node_var.property = child_node.firstChild.nodeValue
                else:
                    print("ERROR:%s解析错误" % (
                        conf_tool.get_config_file_path() + os.path.sep + conf_tool.get_etc_hosts_path()))
                    return None
        return host_node_var

    #
    # 将host对象转换为一个xml host节点
    #
    def __host_to_node(self, host):
        child_node = self.doc.createElement("host")

        ip = self.doc.createElement("ip")
        ip.appendChild(self.doc.createTextNode(host.ip))
        child_node.appendChild(ip)

        domain = self.doc.createElement("domain")
        domain.appendChild(self.doc.createTextNode(host.domain))
        child_node.appendChild(domain)

        hostname = self.doc.createElement("hostname")
        hostname.appendChild(self.doc.createTextNode(host.hostname))
        child_node.appendChild(hostname)

        property = self.doc.createElement("property")
        property.appendChild(self.doc.createTextNode(host.property))
        child_node.appendChild(property)

        return child_node


#
# 将database.conf中定义的etc_hosts=etc_hosts.xml转换为host_set对象
#
def read_etc_hosts():
    doc = minidom.parse((conf_tool.get_base_path() + os.path.sep + conf_tool.get_etc_hosts_path()))
    host_handler = etc_host_handler(doc)
    return host_handler.xml_to_hosts()


#
# 将host_set对象转换为database.conf中定义的etc_hosts=etc_hosts.xml并写入
#
def write_etc_hosts(hosts_set):
    doc = minidom.Document()
    with open((conf_tool.get_base_path() + os.path.sep + conf_tool.get_etc_hosts_path()), "w") as f:
        host_handler = etc_host_handler(doc)
        host_handler.hosts_to_xml(hosts_set).writexml(f)


#
# 获取mds host列表
#
def get_mds_node_list():
    host_set_var = read_etc_hosts()
    mds_node_list = []
    for element in host_set_var.hosts.values():
        if is_mds_node(element):
            mds_node_list.append(element)
    return mds_node_list


#
# 获取oss host列表
#
def get_oss_node_list():
    host_set_var = read_etc_hosts()
    oss_node_list = []
    for element in host_set_var.hosts.values():
        if is_oss_node(element):
            oss_node_list.append(element)
    return oss_node_list


#
# 获取client host列表
#
def get_client_node_list():
    host_set_var = read_etc_hosts()
    client_node_list = []
    for element in host_set_var.hosts.values():
        if is_client_node(element):
            client_node_list.append(element)
    return client_node_list
