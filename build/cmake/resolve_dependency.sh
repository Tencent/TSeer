#!/bin/bash

TIMESTAMP=`date "+%F %T"`

echo "[INFO] $TIMESTAMP resolve depedency rapidjson..."
git clone https://github.com/Tencent/rapidjson.git >/dev/null

if [ "$?" -ne "0" ]; then
    echo "[ERROR] $TIMESTAMP Download rapidjson failed."
    exit 3
fi

mkdir -p ../thirdparty/

cp -rf rapidjson/include/rapidjson ../thirdparty/
rm -rf rapidjson

echo "[SUCC] $TIMESTAMP Successfully resolve dependency"
