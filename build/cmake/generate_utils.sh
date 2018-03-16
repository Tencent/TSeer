#!/bin/bash

tseer_basedir=$1

for item in TseerServer TseerAgent
do 
    mkdir ${tseer_basedir}/Tseer/${item}/util  &>/dev/null
    sed  "s#SEER_BASEDIR=.*#SEER_BASEDIR=${tseer_basedir}#g; s#TARGET=.*#TARGET=${item}#g" ./cmake/start.sh >${tseer_basedir}/Tseer/${item}/util/start.sh
    sed  "s#SEER_BASEDIR=.*#SEER_BASEDIR=${tseer_basedir}#g; s#TARGET=.*#TARGET=${item}#g" ./cmake/stop.sh >${tseer_basedir}/Tseer/${item}/util/stop.sh 
done

cp ./cmake/mon_TseerAgent.sh ${tseer_basedir}/Tseer/TseerAgent/util/mon_TseerAgent.sh &>/dev/null
