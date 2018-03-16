#!/bin/bash

SEER_BASEDIR="/usr/local/"
TARGET="TseerServer"

bin="${SEER_BASEDIR}/Tseer/${TARGET}/bin/${TARGET}"
conf="${SEER_BASEDIR}/Tseer/${TARGET}/conf/${TARGET}.conf"

PID=`ps -eopid,cmd | grep "$bin"| grep "${TARGET}" |  grep -v "grep" |awk '{print $1}'`

if [ "$PID" != "" ]; then
    kill -9 $PID &>/dev/null
fi

ulimit -c unlimited
nohup $bin --config=$conf 2>&1 &
