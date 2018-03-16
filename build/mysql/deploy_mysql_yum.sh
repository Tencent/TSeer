#!/usr/bin/env bash

TIMESTAMP=`date "+%F %T"`

if yum list | grep mysql-server &>/dev/null; then
    yum install mysql mysql-server -y &>/dev/null
    if [ "$?" != "0" ];then
        echo "[ERROR] $TIMESTAMP deploy mysql error."
        exit 2
    fi
    systemctl start mysqld &>/dev/null
    if [ "$?" -ne "0" ];then
        service mysqld start &>/dev/null
        if [ "$?" -ne "0" ]; then
            echo "[ERROR] $TIMESTAMP start mysql error"
            exit 2
        fi
    fi
    echo "[INFO] $TIMESTAMP deploy mysql success"
else
    yum install mysql mariadb-server -y &>/dev/null
    if [ "$?" != "0" ];then
        echo "[ERROR] $TIMESTAMP deploy mysql error."
        exit 2
    fi
    echo "[INFO] $TIMESTAMP deploy mysql success"
    systemctl start mariadb &>/dev/null

    if [ "$?" -ne "0" ];then
        service mysqld start &>/dev/null
        if [ "$?" -ne "0" ]; then
            echo "[ERROR] $TIMESTAMP start mysql error"
            exit 2
        fi
    fi

fi

echo "[INFO] $TIMESTAMP Initial db."
mysql < ./mysql/db.sql &>/dev/null
# create user, if failed, please use root for mysql and execute again
mysql -e "grant all on seer.* to 'seer'@'%' identified by 'seer';"
