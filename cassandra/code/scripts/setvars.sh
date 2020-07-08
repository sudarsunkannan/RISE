#!/bin/bash
set -x

#Pass the release name
export OS_RELEASE_NAME=$1
export PARA="-j40"
export CODE=$PWD
export YCSBHOME=$CODE/mapkeeper/ycsb/YCSB
export CODE=$CODE
export SSD=$HOME/ssd
export SSD_DEVICE="/dev/sdc"
export SSD_PARTITION="/dev/sdc1"
export USER=$USER
export DATASRC=""
export CSRC=$CODE/cassandra

export HOST=localhost
export PORT=9042
export OPSCNT=100000

#Add other servers in the cluster with comma separation
#export SERVERS="128.110.153.215,128.104.222.89,130.127.133.93"
#export SERVERS="128.104.222.222"


export SERVERS=`ifconfig | grep "inet addr" | head -1 | awk '{print $2}' | cut -d ":" -f2`


export LANG="en_US.UTF-8"
export JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF8"
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
export ANT_OPTS="-Xms4G -Xmx4G"
export ANT_HOME=$CODE/apache-ant-1.10.0-bin
export PATH=$PATH:$ANT_HOME/bin


set +x
