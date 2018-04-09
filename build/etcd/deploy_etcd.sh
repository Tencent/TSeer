#!/bin/bash

ETCD_VER=v3.2.5
DOWNLOAD_URL="https://github.com/coreos/etcd/releases/download"
TIMESTAMP=`date "+%F %T"`
ETCD_TMPDIR="/data/tseer_tmp/"
#ETCD_TMPDIR="$HOME/tseer_tmp/"
node_name=$1
node_ip=$2
cluster_port=$3
client_port=$4
cluster_member=$5
ETCD_WORKDIR=$6
CDIR=`pwd`

LOCAL_ETCDFILE=`find $CDIR -name "*etcd*.tar.gz"`
TMP_ETCDFILE=`find $ETCD_TMPDIR -name "*etcd*.tar.gz"`

mkdir -p $ETCD_TMPDIR
mkdir -p ${ETCD_WORKDIR}/{bin,datadir,log}/
cd $ETCD_TMPDIR

echo $LOCAL_ETCDFILE
if [ -n "$LOCAL_ETCDFILE" -a -e "$LOCAL_ETCDFILE" ];then
	echo "[INFO] $TIMESTAMP we has etcd $LOCAL_ETCDFILE"
	cp $LOCAL_ETCDFILE $ETCD_TMPDIR
elif [ -n "$TMP_ETCDFILE" -a -e "$TMP_ETCDFILE" ];then
	echo "[INFO] $TIMESTAMP we has etcd $TMP_ETCDFILE"
else
	LOCAL_ETCDFILE=etcd-${ETCD_VER}-linux-amd64.tar.gz
	echo "[INFO] $TIMESTAMP Download etcd... $LOCAL_ETCDFILE"
    wget -q ${DOWNLOAD_URL}/${ETCD_VER}/${LOCAL_ETCDFILE}
    if [ "$?" -ne "0" ]; then
        echo "[ERROR] $TIMESTAMP Download etcd error!!!"
        exit 1
    fi
fi

# Install etcd
echo "[INFO] $TIMESTAMP Install etcd..."

TGZFILE=$(basename $LOCAL_ETCDFILE)
tar xfz $TGZFILE 

REALFILE=${TGZFILE%.*.*}

cd $REALFILE

cp -rf etcd etcdctl ${ETCD_WORKDIR}/bin/
${ETCD_WORKDIR}/bin/etcd --version
if [ "$?" == "0" ];then
    echo "[INFO] $TIMESTAMP install etcd success"
else
    echo "[ERROR] $TIMESTAMP install etcd failed"
fi


echo "[INFO] $TIMESTAMP Deploy etcd..."
${ETCD_WORKDIR}/bin/etcd --name ${node_name} -data-dir ${ETCD_WORKDIR}/datadir/${node_name}/  \
 -initial-advertise-peer-urls http://${node_ip}:${cluster_port} -listen-peer-urls http://${node_ip}:${cluster_port} \
 -listen-client-urls http://${node_ip}:${client_port} \
 -advertise-client-urls http://${node_ip}:${client_port}  \
 --initial-cluster ${cluster_member}  \
 -initial-cluster-state new &>>${ETCD_WORKDIR}/log/${node_name}.log &
