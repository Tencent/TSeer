#!/bin/sh

installpath=/usr/local/app
etcdcluster=yourmachine:2379
localip=127.0.0.1

bin="$installpath/Seer/SeerServer/bin/SeerServer"
PID=`ps -eopid,cmd | grep "$bin"| grep "SeerServer" |  grep -v "grep" |awk '{print $1}'`

echo $PID

if [ "$PID" != "" ]; then
        kill -9 $PID
        echo "kill -9 $PID"
fi
ulimit -c unlimited


nohup $bin  --etcd=$etcdcluster --localip=$localip --installpath=$installpath 2>&1 &
