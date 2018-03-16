#!/usr/bin/env python
from utils.vssh import execute_script


def install_mysql(install_type, base_dir):
    if install_type == 'yum':
        tmp_rst = execute_script(script="./mysql/deploy_mysql_yum.sh")['localhost']
        stat, rst = tmp_rst['stat'], tmp_rst['rst']

    elif install_type == 'source':
        tmp_rst = execute_script(script="./mysql/deploy_mysql_source.sh", args=base_dir)
        stat, rst = tmp_rst['stat'], tmp_rst['rst']
    else:
        stat, rst = False, 'Unkown install mysql type'

    return stat, rst
