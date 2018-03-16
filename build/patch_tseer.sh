#!/bin/bash

VERSION=1.0.0
TARGET=Tseer-${VERSION}

mkdir -p $TARGET/build

cp tseer_deploy.py $TARGET/build
cp tseer_deploy.ini $TARGET/build
cp webadmin $TARGET/build -rf
cp utils $TARGET/build -rf
cp mysql $TARGET/build -rf
cp etcd $TARGET/build -rf
cp cmake $TARGET/build -rf

if [ ! -f TseerServer/TseerServer/bin/TseerServer ];then 
	echo "please ./build.sh all"
	exit 1
fi
mkdir -p $TARGET/TseerServer


cp ./TseerServer/TseerServer/bin  $TARGET/TseerServer/ -rf
cp ./TseerServer/TseerServer/conf  $TARGET/TseerServer/ -rf

if [ ! -f TseerAgent/TseerAgent/bin/TseerAgent ];then 
	echo "please ./build.sh all"
	exit 1
fi
mkdir -p $TARGET/TseerAgent

cp ./TseerAgent/TseerAgent/bin  $TARGET/TseerAgent -rf
cp ./TseerAgent/TseerAgent/conf  $TARGET/TseerAgent -rf

if [ -d api/cplus/Tseerapi ];then 
	cp api/cplus/Tseerapi $TARGET -rf
fi


tar czf $TARGET.tar.gz $TARGET
echo "tar czf $TARGET.tar.gz $TARGET"
rm $TARGET -rf