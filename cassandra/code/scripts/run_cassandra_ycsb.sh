#!/bin/bash


CASANDARA_SET_ENV() {

    export LANG="en_US.UTF-8"
    export JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF8"
    export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
    export ANT_OPTS="-Xms4G -Xmx4G"
    export ANT_HOME=$CODE/apache-ant-1.9.4
    export PATH=$PATH:$ANT_HOME/bin
}

RUN_YCSB_CASSANDARA() {

    cd $YCSBHOME/cassandra
    $YCSBHOME/cassandra/start_sevice.sh 
}


mkdir $CODE
cd $CODE
CASANDARA_SET_ENV
#Install ycsb and casandara
RUN_YCSB_CASSANDARA
