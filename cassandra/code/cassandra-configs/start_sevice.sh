#!/bin/bash
set -x
YCSB_CLIENT=$CODE/mapkeeper/ycsb/YCSB
OPSCNT=1000000

#Note. for remote host, make the changes in
#/etc/cassandra.yaml for the server to listen 
#on its ip.
HOST=128.110.96.85
PORT=9042
FlushDisk()
{
        sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
        sudo sh -c "sync"
        sudo sh -c "sync"
        sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
}

DESTROY() {
	#Make sure we don't have one already running
	sudo killall java
	sleep 2
}

SETUP() {

	cd $YCSB_CLIENT
	mvn package

	#Assuming cassandra is already installed
	#sudo service cassandra restart
	#Wait for it start
	sleep 5
	#sudo service cassandra status
	$CSRC/bin/nodetool status

	#Open the ports
	sudo ufw allow 9042
	sudo ufw allow 7000

	#Wait
	sleep 4
}

cd $CODE
sleep 10
$CSRC/bin/cassandra
#/usr/local/cassandra/bin/cassandra
sleep 5
#sudo service cassandra status
$CSRC/bin/nodetool status
exit

















#Execute these commands to create ycsb keyspace with cassandra db
cqlsh $HOST -e "create keyspace ycsb WITH REPLICATION = {'class' : 'SimpleStrategy', 'replication_factor': 1 }; USE ycsb; create table usertable (y_id varchar primary key, field0 varchar, field1 varchar, field2 varchar,field3 varchar,field4 varchar, field5 varchar, field6 varchar,field7 varchar,field8 varchar,field9 varchar);" #> ~/output

FlushDisk
#wait
sleep 2
#Warm up phase. Load the db
$YCSBHOME/bin/ycsb load cassandra2-cql -p hosts=$HOST -p port=$PORT -p recordcount=$OPSCNT -P workloads/workloada -s
#Wait
FlushDisk
sleep 5
#Run phase
$YCSBHOME/bin/ycsb run cassandra2-cql -p hosts=localhost -p port=$PORT -p recordcount=$OPSCNT -P workloads/workloada 

sudo service cassandra stop
kill -9 `pidof java`
#DESTROY
FlushDisk
