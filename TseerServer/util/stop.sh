#!/bin/sh

instalpath=/usr/local/app
bin="$installpath/Seer/SeerServer/bin/SeerServer"

PID=`ps -eopid,cmd | grep "$bin"| grep "SeerServer" |  grep -v "grep" |awk '{print $1}'`

echo $PID

if [ "$PID" != "" ]; then
        kill -9 $PID
            echo "kill -9 $PID"
        fi