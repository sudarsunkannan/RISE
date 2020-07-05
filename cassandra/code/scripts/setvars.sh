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

#Add other servers in the cluster with comma separation
#export SERVERS="128.110.153.215,128.104.222.89,130.127.133.93"
#export SERVERS="128.104.222.222"
export SERVERS=`ifconfig | grep "inet addr" | head -1 | awk '{print $2}' | cut -d ":" -f2`
set +x
