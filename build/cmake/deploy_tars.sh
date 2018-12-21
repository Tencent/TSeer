#!/bin/bash

TIMESTAMP=`date "+%F %T"`

if [ -n "Tars" -a -e "Tars" ];then
	echo "[INFO] $TIMESTAMP we have already downloaded tars"
else
    echo "[INFO] $TIMESTAMP downloading tars..."
    git clone --recursive  https://github.com/Tencent/Tars.git >/dev/null
    if [ "$?" -ne "0" ]; then
        echo "[ERROR] $TIMESTAMP Download Tars failed."
        exit 2
    fi
fi


# process Tars source file
rm -f ./Tars/cpp/util/src/tc_gzip.cpp
rm -f ./Tars/cpp/util/src/tc_mysql.cpp 
sed -i '/framework/d' ./Tars/cpp/CMakeLists.txt 
sed -i '/INSTALL/d' ./Tars/cpp/CMakeLists.txt 
sed -i '/test/d' ./Tars/cpp/CMakeLists.txt 

# building
mkdir -p ../thirdparty
INSTALLTARSPATH=$(pwd)/../thirdparty/tars/

TIMESTAMP=`date "+%F %T"`
echo "[INFO] $TIMESTAMP building tars into $INSTALLTARSPATH ..."
cd ./Tars/cpp && cmake . -DCMAKE_INSTALL_PREFIX=$INSTALLTARSPATH && make && make install;
if [ "$?" -ne "0" ]; then
	echo "[ERROR] $TIMESTAMP Compile Tars failed."
	exit 3
fi

rm -rf Tars

echo "[SUCC] $TIMESTAMP Successfully install tars in $INSTALLTARSPATH"

