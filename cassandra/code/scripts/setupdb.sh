#!/bin/bash
#Script to compile Cassandra from source
set -x

CASANDARA_SET_ENV() {

    export LANG="en_US.UTF-8"
    export JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF8"
    export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
    export ANT_OPTS="-Xms4G -Xmx16G"
    export ANT_HOME=$CODE/apache-ant-1.9.4
    export PATH=$PATH:$ANT_HOME/bin
}

DESTROY() {
	#Make sure we don't have one already running
	sudo killall java
	sudo killall java
	sleep 2
	sudo killall java
	sleep 2
	sudo killall java
	sleep 2
}

SETUP() {
	cd $CODE/mapkeeper/ycsb/YCSB
	mvn package

	#Assuming cassandra is already installed
	sudo service cassandra restart
	#Wait for it start
	sleep 5
	sudo service cassandra status
	sudo nodetool status

	#Open the ports
	sudo ufw allow 9042
	sudo ufw allow 7000

	#Wait
	sleep 4
}

mkdir $CODE
cd $CODE
DESTROY
kill -9 `pidof java`
CASANDARA_SET_ENV
SETUP
