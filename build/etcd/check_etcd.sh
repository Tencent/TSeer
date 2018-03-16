#!/usr/bin/env bash

TIMESTAMP=`date "+%F %T"`
ETCD_WORKDIR=$1
host=$2

stat=`curl -s -m 5 ${host}:2379/health |grep 'true'`
if [ -z "$stat" ]; then
    echo "[ERROR] $TIMESTAMP check etcd failed, see log ${ETCD_WORKDIR}/log"
    exit 1
else
    echo "[SUCC] $TIMESTAMP Deploy and check etcd success."
fi
