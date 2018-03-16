#!/bin/bash

SEER_BASEDIR=$1
TIMESTAMP=`date "+%F %T"`
# start Tseer server
chmod +x $SEER_BASEDIR/Tseer/TseerServer/util/stop.sh 
chmod +x $SEER_BASEDIR/Tseer/TseerServer/util/start.sh
sh $SEER_BASEDIR/Tseer/TseerServer/util/stop.sh
sh $SEER_BASEDIR/Tseer/TseerServer/util/start.sh

if [ "$?" -ne "0" ]; then
    echo "[ERROR] $TIMESTAMP starting TseerServer failed."
    exit 1
fi

sleep 5
PID=`ps -eopid,cmd | grep "TseerServer"| grep -v "grep" |awk '{print $1}'`

if [ "$PID" != "" -a -f $SEER_BASEDIR/Tseer/TseerAgent/bin/TseerAgent ];then
chmod +x $SEER_BASEDIR/Tseer/TseerAgent/util/stop.sh
chmod +x $SEER_BASEDIR/Tseer/TseerAgent/util/start.sh
sh $SEER_BASEDIR/Tseer/TseerAgent/util/stop.sh
sh $SEER_BASEDIR/Tseer/TseerAgent/util/start.sh
fi

