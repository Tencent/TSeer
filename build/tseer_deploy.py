#!/usr/bin/env python
import ConfigParser
import datetime
import sys
import time
from commands import *

from utils.vssh import execute_script
from utils.get_os_info import get_ip

tseer_conf = ConfigParser.ConfigParser()
tseer_conf.read('tseer_deploy.ini')
TIME_STAMP = str(datetime.datetime.now()).split('.')[0]


def config_parse(section, option, default=None):
    try:
        value = tseer_conf.get(section, option)
    except ConfigParser.NoOptionError:
        value = default
    return value


def deploy_etcd():
    from etcd.deploy_etcd import install_etcd
    print "[INFO] %s Starting install etcd..." % TIME_STAMP
    host_list = config_parse('etcd', 'host_list', 'localhost')
    basedir = config_parse('etcd', 'base_dir', '/data/etcd/')
    client_port = config_parse('etcd', 'client_port', 2379)
    cluster_port = config_parse('etcd', 'cluster_port', 2380)
    stat, rst = install_etcd(host_list, cluster_port = int(cluster_port), client_port = int(client_port), node_prefix='tseer_etcd', basedir=basedir)
    if not stat:
        print "[ERROR] %s deploy etcd error.\ndetail: " % TIME_STAMP
        for host, info in rst.items():
            print '%s: %s\n' % (host, info['rst'])
        sys.exit(1)


def deploy_mysql():
    from mysql.deploy_mysql import install_mysql
    print "[INFO] %s Starting install mysql..." % TIME_STAMP
    install_type = config_parse('mysql', 'install_type', 'yum')
    base_dir = config_parse('mysql', 'base_dir', '/usr/local/mysql')
    stat, rst = install_mysql(install_type, base_dir)
    if not stat:
        print "[INFO] %s Deploy mysql error.\ndetail: " % (TIME_STAMP, rst)
        sys.exit(1)


def local_install():
    print "[INFO] %s Starting local install tseer..." % TIME_STAMP
    tseer_basedir = config_parse('tseer', 'base_dir', '/usr/local/')
    tmp_rst = execute_script(script='./cmake/local_install.sh', args=tseer_basedir)['localhost']
    stat, rst = tmp_rst['stat'], tmp_rst['rst']
    if not stat:
        sys.exit(1)


def cmake_build():
    print "[INFO] %s Starting build tseer..." % TIME_STAMP
    tseer_basedir = config_parse('tseer', 'base_dir', '/usr/local/')
    storage = config_parse('tseer', 'storage', 'etcd')
    cmake_cmd = '-DINSTALL_PATH=%s ' % tseer_basedir
    if storage == 'mysql':
        mysql_dir = config_parse('mysql', 'base_dir', '/usr/local/mysql')
        cmake_cmd += '-DUSE_MYSQL=ON -DMYSQL_DIR=%s' % mysql_dir
    tmp_rst = execute_script(script='./cmake/make_install.sh', args=cmake_cmd)['localhost']
    stat, rst = tmp_rst['stat'], tmp_rst['rst']
    if not stat:
        sys.exit(1)


def resolve_dependency():
    print '[INFO] %s Starting resolve dependency...' % TIME_STAMP
    tmp_rst = execute_script(script='./cmake/resolve_dependency.sh')['localhost']
    stat, rst = tmp_rst['stat'], tmp_rst['rst']
    print rst
    if not stat:
        sys.exit(1)


def deploy_webadmin():
    pass


def generate_utils():
    tseer_basedir = config_parse("tseer", "base_dir", '/usr/local/')
    tmp_rst = execute_script(script='./cmake/generate_utils.sh', args=tseer_basedir)
    rst = tmp_rst['localhost']['rst']


def change_sc_or_conf():
    print "[INFO] %s Change conf..." % TIME_STAMP
    localip = get_ip()
    tseer_basedir = config_parse('tseer', 'base_dir', '/usr/local/')
    storage = config_parse('tseer', 'storage', 'etcd')
    etcd_host = config_parse('etcd', 'host_list', localip).split(',')
    etcd_client_port = config_parse('etcd', 'client_port', '2379')
    etcd_endpoint = ';'.join([i+':'+etcd_client_port for i in etcd_host])
    bind_ip = config_parse('tseer', 'bind_ip', localip)
    if bind_ip in ['localhost', '127.0.0.1']:
        bind_ip = get_ip()
    regport = config_parse('tseer_server', 'regport', '9902')
    queryport = config_parse('tseer_server', 'queryport', '9003')
    apiport = config_parse('tseer_server', 'apiport', '9904')
    args = "%s %s %s %s %s %s %s" % (
        tseer_basedir, bind_ip, storage, etcd_endpoint,
        regport, queryport, apiport)
    tmp_rst = execute_script(script='./cmake/change_conf.sh', args=args)['localhost']
    stat, rst = tmp_rst['stat'], tmp_rst['rst']
    if not stat:
        sys.exit(1)


def start_server():
    import subprocess
    print '[INFO] %s Starting server...' % TIME_STAMP
    tseer_basedir = config_parse('tseer', 'base_dir', '/usr/local/')
    stat = subprocess.call(['./cmake/start_server.sh', tseer_basedir])



def start_agent():
    import subprocess
    print '[INFO] %s Starting agent...' % TIME_STAMP
    tseer_basedir = config_parse('tseer', 'base_dir', '/usr/local/')
    stat = subprocess.call(['./cmake/start_agent.sh', tseer_basedir])

def deploy_tars():
    print '[INFO] %s Starting deploy tars...' % TIME_STAMP
    tmp_rst = execute_script(script='./cmake/deploy_tars.sh')['localhost']
    stat, rst = tmp_rst['stat'], tmp_rst['rst']
    if not stat:
        sys.exit(1)
    else:
        print '[INFO] %s Deploy tars success!' % TIME_STAMP


def main():
    deploy_tars()

    print "[INFO] %s Starting deploy tseer..." % TIME_STAMP
    install_type = config_parse('tseer', 'install_type', 'bin')
    storage = config_parse('tseer', 'storage', 'etcd')
    
    deploy_etcd()

    if install_type == "bin":
        local_install()
    else:
        resolve_dependency()
        cmake_build()

    deploy_webadmin()
    generate_utils()
    change_sc_or_conf()

    start_server()
    time.sleep(2)
    start_agent()
    print "[SUCC] %s Successfully deploy tseer. Enjoy it~" % TIME_STAMP
    print "[SUCC] %s If there is something wrong, please check seer_make.log~" % TIME_STAMP


if __name__ == '__main__':
    main()


