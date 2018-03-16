# coding:utf8

import socket


def get_ip():
    return socket.gethostbyname(socket.gethostname())