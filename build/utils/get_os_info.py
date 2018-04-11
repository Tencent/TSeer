# coding:utf8

import socket
import fcntl
import struct


def get_ip(ifname='eth0'):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ad = socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915, # SIOCGIFADDR
        struct.pack('256s', ifname[:15])
        )[20:24])
    s.close()
    return ad

   
if __name__ == '__main__':
    print get_ip()

