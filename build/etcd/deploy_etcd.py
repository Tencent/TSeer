#!/usr/bin/env python
# coding:utf8
from commands import *
from utils.get_os_info import get_ip
from utils.vssh import execute_script
import time

def linenumber():
	from inspect import currentframe
	cf = currentframe()
	return "%s:%s" % (__file__,cf.f_back.f_lineno)

def install_etcd(iplist='localhost', cluster_port=2380, client_port=2379, node_prefix='test', basedir='/usr/local/etcd/'):
    if iplist == 'localhost':
        remote_run = False
        cluster_port_list = [cluster_port, cluster_port + 1000, cluster_port + 2000]
        client_port_list = [client_port, client_port + 1000, client_port + 2000]
        local_ip = get_ip()
        iplist = [local_ip for _ in xrange(3)]
    else:
        remote_run = True
        iplist = iplist.split(';')
        cluster_port_list = [2380 for _ in xrange(len(iplist))]
        client_port_list = [2379 for _ in xrange(len(iplist))]

    cluster_member = ','.join(["%s%s=http://%s:%s" % (node_prefix, i[0], i[1], i[2]) for i in zip(range(len(iplist)), iplist, cluster_port_list)])
    rst = {}
    for i in xrange(len(iplist)):
        args = " {node_prefix}{id} {node_ip} {cluster_port} {client_port} {cluster_member} {etcd_basedir}".format(
            node_prefix=node_prefix, id=i, node_ip=iplist[i],
            cluster_port=cluster_port_list[i], client_port=client_port_list[i],
            cluster_member=cluster_member,
            etcd_basedir=basedir
        )
        item_rst = execute_script(script="./etcd/deploy_etcd.sh", args=args, remote=remote_run, ip_list=[iplist[i], ])
        for k, v in item_rst.items():
            if not v['stat']:
                rst[k] = v
    #check the etcd installing 
    args = "%s %s" % (basedir,get_ip())
    time.sleep(3)
    tmp_rst = execute_script(script='./etcd/check_etcd.sh',args=args)['localhost']
    stat, chk_rst = tmp_rst['stat'], tmp_rst['rst']
    #if not stat:
    #    return False, rst
    return True, chk_rst


if __name__ == '__main__':
    print install_etcd('localhost', node_prefix='seer_etcd')

