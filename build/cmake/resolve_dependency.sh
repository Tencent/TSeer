#!/bin/bash

TIMESTAMP=`date "+%F %T"`

echo "[INFO] $TIMESTAMP resolve depedency rapidjson..."
git clone https://github.com/Tencent/rapidjson.git &>/dev/null

if [ "$?" -ne "0" ]; then
    echo "[ERROR] $TIMESTAMP Download rapidjson failed."
    exit 3
fi

cp -rf rapidjson/include/rapidjson ../thirdparty/rapidjson
rm -fr rapidjson

echo "[SUCC] $TIMESTAMP Successfully resolve dependency"
