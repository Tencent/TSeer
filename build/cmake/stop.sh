#!/bin/bash


SEER_BASEDIR="/usr/local/"
TARGET="TseerServer"

bin="${SEER_BASEDIR}/Tseer/${TARGET}/bin/${TARGET}"

PID=`ps -eopid,cmd | grep "$bin"| grep "${TARGET}" |  grep -v "grep" |awk '{print $1}'`

if [ "$PID" != "" ]; then
    kill -9 $PID
fi
