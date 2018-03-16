#!/bin/bash

TIMESTAMP=`date "+%F %T"`
seer_basedir=$1

mkdir -p $seer_basedir/Tseer/ &>/dev/null

current_path=`pwd`
current_dir=`dirname $current_path`

if [ "$current_dir" == "$1/Tseer" ];then
    echo "[ERROR] $TIMESTAMP install path and source at same path"
    exit 1
fi

rm -fr $seer_basedir/Tseer/{api,TseerServer,TseerAgent}
cp -fr ../{api,TseerServer,TseerAgent} $seer_basedir/Tseer &>./seer_make.log
cp ./cmake/installscript.tgz ${seer_basedir}/Tseer/TseerServer/bin/

if [ "$?" -ne 0 ]; then
    echo "[ERROR] $TIMESTAMP local install seer failed, see log seer_make.log."
    exit 1
fi
chmod a+x ${seer_basedir}/Tseer/TseerServer/bin/TseerServer
echo "[SUCC] $TIMESTAMP Successfully install seer."
