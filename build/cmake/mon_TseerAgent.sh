#!/bin/bash

IP=`/sbin/ip a s |awk '/inet/ { if (($2 !~ /127.0.0.1/) && ($2 !~ /::1/)) { print $2} }'| cut -d / -f 1 | head -n 1`
PORT=9765
BASE_DIR=/usr/local/

# if you are good admin, you should add alert here!

stop_server()
{
        bash ${BASE_DIR}/Tseer/TseerAgent/util/stop.sh
        sleep 2
        bash ${BASE_DIR}/Tseer/TseerAgent/util/stop.sh
        ps -ef |grep "${BASE_DIR}/Tseer/TseerAgent/bin/TseerAgent"|grep -v grep >/dev/null 2>&1
        if test $? = 0; then
                bash ${BASE_DIR}/Tseer/TseerAgent/util/stop.sh
                sleep 1
                ps -ef |grep "${BASE_DIR}/Tseer/TseerAgent/bin/TseerAgent"|grep -v grep >/dev/null 2>&1
                if test $? = 0; then
                        echo "stop fail"
                fi
        fi
        return 0
}

start_server()
{
        bash ${BASE_DIR}/Tseer/TseerAgent/util/start.sh
        sleep 5
        ps -ef |grep  "${BASE_DIR}/Tseer/TseerAgent/bin/TseerAgent"|grep -v grep >/dev/null 2>&1
        if [ "$?" -ne "0" ]; then
                return 1
        fi
        return 0
}

mon_process()
{
        ps -ef |grep  "${BASE_DIR}/Tseer/TseerAgent/bin/TseerAgent"|grep -v grep >/dev/null 2>&1
        if test $? = 1; then
                echo "`date +%F\ %T` the TseerAgent process is not exist"
                return 1
        else
                return 0
        fi
}

mon_port()
{
        netstat -tnl | grep "$IP:$PORT" >/dev/null 2>&1
        if test $? = 0; then
                echo "`date +"%Y-%m-%d %H:%M:%S"` the port $PORT is good"
                return 0
        else
                echo "`date +"%Y-%m-%d %H:%M:%S"` the port $PORT is not answer"
                return 1
        fi
}

for i in {1..11}
do
        mon_process
        flag_process=$?
        if test $flag_process = 1; then
                stop_server
                start_server
        else
                mon_port
                flag_port=$?
                if test $flag_port = 1;then
                        stop_server
                        start_server
                else
                        echo "TseerAgent process and port state is good"
                fi
        fi
        sleep 5
done
exit 0