#!/bin/bash

SEER_BASEDIR=$1
TIMESTAMP=`date "+%F %T"`
if [ -f $SEER_BASEDIR/Tseer/TseerAgent/bin/TseerAgent ];then
chmod +x $SEER_BASEDIR/Tseer/TseerAgent/util/stop.sh
chmod +x $SEER_BASEDIR/Tseer/TseerAgent/util/start.sh
sh $SEER_BASEDIR/Tseer/TseerAgent/util/stop.sh
sh $SEER_BASEDIR/Tseer/TseerAgent/util/start.sh
fi

if [ "$?" -ne "0" ]; then
    echo "[ERROR] $TIMESTAMP starting TseerAgent failed."
    exit 1
fi
