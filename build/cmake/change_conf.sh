#!/bin/bash

seer_basedir=$1
local_ip=$2
storage=$3
etcd_endpoint=$4
regport=$5
queryport=$6
apiport=$7

TIMESTAMP=`date "+%F %T"`
# edit TseerServer config

sed -i "s#installpath=.*#installpath=${seer_basedir}#g" ${seer_basedir}/Tseer/TseerServer/conf/TseerServer.conf
sed -i "s#localip=.*#localip=${local_ip}#g" ${seer_basedir}/Tseer/TseerServer/conf/TseerServer.conf
sed -i "s#store=.*#store=${storage}#g" ${seer_basedir}/Tseer/TseerServer/conf/TseerServer.conf
sed -i "s#host=.*#host=${etcd_endpoint}#g"  ${seer_basedir}/Tseer/TseerServer/conf/TseerServer.conf
sed -i "s#regport=.*#regport=${regport}#g" ${seer_basedir}/Tseer/TseerServer/conf/TseerServer.conf
sed -i "s#queryport=.*#queryport=${queryport}#g" ${seer_basedir}/Tseer/TseerServer/conf/TseerServer.conf
sed -i "s#apiport=.*#apiport=${apiport}#g" ${seer_basedir}/Tseer/TseerServer/conf/TseerServer.conf
# edit installscript
sed -i "s#download.seer.wsd.com#${local_ip}:${apiport}#" ${seer_basedir}/Tseer/TseerServer/bin/installscript.tgz

if [ "$?" -ne "0" ];then
    echo "[ERROR] $TIMESTAMP Change conf failed."
    exit 1
fi


# edit TseerAgent config
sed -i "s#installpath=.*#installpath=${seer_basedir}#g" ${seer_basedir}/Tseer/TseerAgent/conf/TseerAgent.conf
sed -i "s#localip=.*#localip=${local_ip}#g" ${seer_basedir}/Tseer/TseerAgent/conf/TseerAgent.conf
sed -i "s#locator=.*#locator=Tseer.TseerServer.QueryObj@tcp -h ${local_ip} -p ${queryport}#g" ${seer_basedir}/Tseer/TseerAgent/conf/TseerAgent.conf


if [ "$?" -ne "0" ];then
    echo "[ERROR] $TIMESTAMP Change conf failed."
    exit 1
fi

echo "[SUCC] $TIMESTAMP Change conf successfully."
