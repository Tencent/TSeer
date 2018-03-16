#!/bin/bash

TIMESTAMP=`date "+%F %T"`
cmake_args=$@

install_path=`echo $cmake_args | tr -s ' ' '\n' | awk -F= '/INSTALL/{print $2}'`

echo "[INFO] $TIMESTAMP start build tseer..."
yum install gcc make gcc-c++ -y &>/dev/null
cd ..
cmake . ${cmake_args} &>>./build/tseer_make.log

if [ "$?" -ne 0 ]; then
    echo "[ERROR] $TIMESTAMP cmake build seer failed, see log tseer_make.log."
    exit 1
fi

chmod a+x thirdparty/tars/tools/tars2cpp
make &>>./build/tseer_make.log
make install &>>./build/tseer_make.log

if [ "$?" -ne 0 ]; then
    echo "[ERROR] $TIMESTAMP make install seer failed, see log tseer_make.log."
    exit 1
fi
echo "[INFO] $TIMESTAMP succ install tseer."

# cp installscript to TseerServer
mkdir -p ${install_path}/Tseer/TseerServer/{bin,conf}
cp TseerServer/TseerServer/conf/TseerServer.conf ${install_path}/Tseer/TseerServer/conf/
cp TseerServer/TseerServer/bin/TseerServer ${install_path}/Tseer/TseerServer/bin/
cp build/cmake/installscript.tgz ${install_path}/Tseer/TseerServer/bin/

# build api
cd api/cplus/src/
mkdir -p  ${install_path}/Tseer/api/cplus/{include,lib} 
cmake . &>/dev/null
make &>/dev/null
cd -
cd api/cplus/Tseerapi/lib/
cp Tseer_api.h Tseer_comm.h ${install_path}/Tseer/api/cplus/include/
cp libtseerapi.a ${install_path}/Tseer/api/cplus/lib/

# buid java api

# build agent
cd - && cd TseerAgent/TseerAgent
mkdir -p ${install_path}/Tseer/TseerAgent/{bin,conf,utils}
cp -fr bin/TseerAgent ${install_path}/Tseer/TseerAgent/bin/
cp -fr conf/TseerAgent.conf ${install_path}/Tseer/TseerAgent/conf/

echo "[SUCC] $TIMESTAMP Successfully install seer."
