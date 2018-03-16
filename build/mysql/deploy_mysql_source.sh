#!/usr/bin/env bash

TIMESTAMP=`date "+%F %T"`
INSTALL_DIR=$1
MYSQL_TMPDIR='/data/seer_tmp/'

ORI_DIR=`pwd`
mkdir -p $TMP_DIR && cd $TMP_DIR

echo "[INFO] $TIMESTAMP Download mysql"
if [ ! -e ${MYSQL_TMPDIR}/mysql-5.6.37.tar.gz ];then
    wget -q https://cdn.mysql.com//Downloads/MySQL-5.6/mysql-5.6.37.tar.gz
    if [ "$?" -ne "0" ]; then
        echo "[ERROR] $TIMESTAMP Download mysql error!!!"
        exit 1
    fi
fi

echo  "[INFO] $TIMESTAMP Resolve dependencies... "
yum install -y ncurses-devel bison openssl-devel \
cmake gcc make gcc-c++ libmcrypt*  &> /dev/null

[ $? -eq 0 ] &&  echo "[INFO] $TIMESTAMP Untar mysql source code...." || {
	echo "[ERROR] $TIMESTAMP Resolve dependencies Failure..."
	exit 1
}

tar zxf mysql-5.6.37.tar.gz
cd mysql-5.6.37/

cmake  \
-DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
-DSYSCONFIGDIR=/etc \
-DMYSQL_TCP_PORT=3306 \
-DMYSQL_UNIX_ADDR=/tmp/mysql.sock \
-DMYSQL_USER=mysql \
-DDEFAULT_CHARSET=utf8 \
-DEXTRA_CHARSET=all \
-DDEFAULT_COLLATION=utf8_general_ci \
-DWITH_READLINE=1 \
-DWITH_SSL=system \
-DWITH_EMBEDDED_SERVER=1 \
-DENABLED_LOCAL_INFILE=1 \
-DWITH_INNOBASE_STORAGE_ENGINE=1 &>/dev/null

if [ "$?" -eq "0" ]; then
	echo "[INFO] $TIMESTAMP Making and install Mysql..."
	make &>/dev/null
	make install  &>/dev/null
	if [ "$?" -ne "0" ]; then
        echo "[ERROR] $TIMESTAMP install mysql error."
        exit 2
	fi
else
	echo "[ERROR] $TIMESTAMP Cmake Failure."
	exit 2
fi

echo "[INFO] $TIMESTAMP Create mysql user..."
grep -q mysql /etc/passwd
[ $? -eq 0 ]  || useradd -M -s /sbin/nologin mysql

echo "[INFO] $TIMESTAMP Initial mysql..."
cd $INSTALL_DIR
chown  -R  mysql:mysql .
./scripts/mysql_install_db  --user=mysql  --basedir=${INSTALL_DIR} --datadir=${INSTALL_DIR}/data &>/dev/null
cp  ./support-files/mysql.server /etc/init.d/mysqld
chmod  +x /etc/init.d/mysqld

/etc/init.d/mysqld start &>/dev/null
if [ "$?" -ne "0" ];then
    echo "[ERROR] $TIMESTAMP start mysql failed"
    exit 2
else
    echo "[Succ] $TIMESTAMP start mysql successfully"
fi

ln -fs /tmp/mysql.sock /var/lib/mysql/mysql.sock

cat <<EOF >> /etc/profile
MYSQL_EXE_PATH=${INSTALL_DIR}/bin
export PATH=$PATH:$MYSQL_EXE_PATH
EOF
source /etc/profile
echo "[Succ] $TIMESTAMP Install Mysql successfully"

# set user and password
# import data and stuct
echo "[Succ] $TIMESTAMP initial db..."
cd $ORI_DIR
mysql < ./mysql/db.sql &>/dev/null
# create user, if failed, please use root for mysql and execute again
mysql -e "grant all on seer.* to 'seer'@'%' identified by 'seer';"
