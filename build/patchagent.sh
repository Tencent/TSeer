#!/bin/sh
TARGET=TseerAgent

if [ -d ${TARGET}_tmp_dir ]; then 
	echo "dir has exist:${TARGET}_tmp_dir, abort."; 
	exit 1; 
else 
	mkdir -p ${TARGET}_tmp_dir/Tseer/${TARGET}/bin;
	mkdir -p ${TARGET}_tmp_dir/Tseer/${TARGET}/conf;
	mkdir -p ${TARGET}_tmp_dir/Tseer/${TARGET}/data;
	mkdir -p ${TARGET}_tmp_dir/Tseer/${TARGET}/util;
	cp -rf ./TseerAgent/TseerAgent/bin/${TARGET} ${TARGET}_tmp_dir/Tseer/${TARGET}/bin/; 
	cp -rf ./cmake/start.sh ${TARGET}_tmp_dir/Tseer/${TARGET}/util/;
	sed -i "s#TARGET=.*#TARGET=${TARGET}#g" ${TARGET}_tmp_dir/Tseer/${TARGET}/util/start.sh
	cp -rf ./cmake/stop.sh ${TARGET}_tmp_dir/Tseer/${TARGET}/util/; 
	sed -i "s#TARGET=.*#TARGET=${TARGET}#g" ${TARGET}_tmp_dir/Tseer/${TARGET}/util/stop.sh
	cp -rf ./cmake/mon_TseerAgent.sh ${TARGET}_tmp_dir/Tseer/${TARGET}/util/; 
	cp -rf ./TseerAgent/TseerAgent/conf/TseerAgent.conf ${TARGET}_tmp_dir/Tseer/${TARGET}/conf/; 
	cd ${TARGET}_tmp_dir; tar  -czvf ${TARGET}.tgz Tseer/; cd ..; 
	echo "`pwd`"
	VERSION=`strings ${TARGET}_tmp_dir/Tseer/${TARGET}/bin/${TARGET} | grep "TseerAgent_" | head -1|awk -F_ '{print $2 "_" $3}'`; 
	mv ${TARGET}_tmp_dir/${TARGET}.tgz ./${TARGET}.tgz;  
	MD5=`md5sum ${TARGET}.tgz | awk '{print $1}'`;
	TIME=`date +%Y%m%d%H%M%S`;
	mv ./${TARGET}.tgz ./${TARGET}_${TIME}_${VERSION}_${MD5}.tgz;
	rm -rf ${TARGET}_tmp_dir;
	echo ${TARGET}_${TIME}_${VERSION}_${MD5}.tgz;
fi