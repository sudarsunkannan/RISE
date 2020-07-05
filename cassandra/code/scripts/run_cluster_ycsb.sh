#!/bin/bash


CASANDARA_SET_ENV() {

    export LANG="en_US.UTF-8"
    export JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF8"
    export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
    export ANT_OPTS="-Xms4G -Xmx4G"
    export ANT_HOME=$CODE/apache-ant-1.9.4
    export PATH=$PATH:$ANT_HOME/bin
}

SETUP() {
    sudo service cassandra stop
    sudo rm -rf /var/lib/cassandra/data/system/*
    
    if [ ! -f "$HOME/.ssh/id_rsa.pub" ]; then
        ssh-keygen
        cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys
    fi	
    #sudo cat "-A INPUT -p tcp -s $SERVERS -m multiport --dports 7000,9042 -m state --state NEW,ESTABLISHED -j ACCEPT" > rules.v4
    #sudo cp rules.v4 /etc/iptables/
    sed -i '/seeds:/c\        - seeds: '"\"$SERVERS\""'' $CODE/scripts/cassandra.yaml 

    hostname=`hostname -i`
    echo $hostname

    #Change listen address and RPC address
    sed -i '/listen_address:/c\listen_address: '"$hostname"'' $CODE/scripts/cassandra.yaml 
    sed -i '/rpc_address:/c\rpc_address: '"$hostname"'' $CODE/scripts/cassandra.yaml 

    #Take a backup and copy modified 
    sudo cp /etc/cassandra/cassandra.yaml /etc/cassandra/cassandra_orig.yaml
    sudo cp $CODE/scripts/cassandra.yaml /etc/cassandra/cassandra.yaml

    sudo service cassandra start
    sleep 10
    sudo nodetool status
}


RUN_YCSB_CASSANDARA() {

    cd $YCSBHOME/cassandra
    ./start_sevice.sh 
}


mkdir $CODE
cd $CODE
CASANDARA_SET_ENV
SETUP
#Install ycsb and casandara
#RUN_YCSB_CASSANDARA
