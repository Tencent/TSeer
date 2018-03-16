#!/usr/bin/env python
# coding:utf8

import os
import sys
from commands import *

from seer_exceptions import *


def execute_script(script, args=' ', remote=False, ip_list=None):
    if remote:
        ssh_obj = Vssh(ip_list)
        return ssh_obj.remote_execute_scipt(script, args)
    else:
        stat, rst = run_local_script(script, args)
        return {'localhost': {'stat': stat, 'rst': rst}}


class Vssh(object):
    def __init__(self, ip_list, port=22):
        import paramiko
        self.port = port
        self.ip_list = ip_list
        self.ssh = paramiko.SSHClient()
        self.ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        if not os.path.exists('./../hosts'):
            raise FileNotExist('hosts')

        self.host_info = []
        with open('./../hosts') as f:
            for i in f.readlines():
                if not i.startswith('#'):
                    self.host_info.append(i.split())

    def get_user_passwd(self, host):
        for ip, user, passwd in self.host_info:
            if ip == host:
                return user, passwd

    def remote_execute_scipt(self, script, args='', interpreter='sh'):
        rst = {}
        cmd = '%s %s %s' %(interpreter, args, script)
        origin_ip_list = self.ip_list
        upload_rst = self.upload_script(script, script)
        for ip, urst in upload_rst.items():
            if not urst['stat']:
                self.ip_list.remove()
                rst[ip] = urst

        run_rst = self.run_remote_script(cmd)
        for i, v in run_rst.items():
            rst[i] = v
        self.ip_list = origin_ip_list
        return rst

    def upload_script(self, local_file, remote_file):
        rst = {}
        for ip in self.ip_list:
            user, passwd = self.get_user_passwd(ip)
            try:
                self.ssh.connect(hostname=ip, port=self.port, username=user, password=passwd)
                sftp = self.ssh.open_sftp()
                sftp.put(local_file, remote_file)
                stat = True
                ret_msg = 'Success'
            except Exception as e:
                ret_msg = str(e)
                stat = False
            rst[ip] = {'stat': stat, 'rst': ret_msg}
        return rst

    def run_remote_script(self, cmd):
        rst = {}

        for ip in self.ip_list:
            user, passwd = self.get_user_passwd(ip)
            try:
                self.ssh.connect(hostname=ip, port=self.port, username=user, password=passwd)
                stdin, stdout, stderr = self.ssh.exec_command(cmd)
                stat = True
                ret_msg = stdout.read() + '\n' + stderr.read()
            except Exception as e:
                ret_msg = str(e)
                stat = False
            rst[ip] = {'stat': stat, 'rst': ret_msg}
        return rst


def run_local_script(script, args, interpreter='sh'):
    cmd = '%s %s %s' % (interpreter, script, args)
    stat, rst = getstatusoutput(cmd)
    return stat == 0, rst
