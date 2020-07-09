
### Set environmental variable
```
git clone https://github.com/SudarsunKannan/RISE
cd RISE/cassandra/code
source scripts/setvars.sh
```


### Install system libraries
```
$CODE/scripts/install_libs.sh
```

### Install JAVA 8

```
sudo add-apt-repository ppa:openjdk-r/ppa -y
sudo apt-get update
sudo apt-get install openjdk-8-jdk -y
echo "Now Set the default JAVA version to Open JDK 8 (possibly listed as  /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java)"
sudo update-alternatives --config java
java -version //Make sure version number is reported as JAVA 8 something similar to the below output
```

SAMPLE Output of "java -version"
```
Picked up JAVA_TOOL_OPTIONS: -Dfile.encoding=UTF8
openjdk version "1.8.0_252"
OpenJDK Runtime Environment (build 1.8.0_252-8u252-b09-1~18.04-b09)
OpenJDK 64-Bit Server VM (build 25.252-b09, mixed mode)
```


### Install CASSANDRA binary
```
cd $CODE	
mkdir $CODE/cassandra
sudo apt-key adv --keyserver pool.sks-keyservers.net --recv-key A278B781FE4B2BDA
sudo apt-get update
sudo apt-get install -y --force-yes cassandra
```

### Install Compiler for CASSANDRA source
```
sudo apt-get update
sudo apt-get install -y git tar g++ make automake autoconf libtool  wget patch libx11-dev libxt-dev pkg-config texinfo locales-all unzip python
sudo apt-get install -y maven
wget http://archive.apache.org/dist/ant/binaries/apache-ant-1.10.0-bin.tar.gz
tar -xvf apache-ant-1.10.0-bin.tar.gz
sudo apt-get install ant
cp $CODE/xml/libraries.properties $CODE/apache-ant-1.10.0/lib/libraries.properties
```

### Now we are going to compile Cassandra from source 

```
cd $CODE
git clone https://github.com/apache/cassandra.git
cd cassandra
git checkout cassandra-3.11.2 
sed  -i 's/Xss256k/Xss32m/g' build.xml conf/jvm.options


//Cassandra downloads packages during installation.
//So, let's point Cassandra to right Apache packages.

cp $CODE/xml/*.xml $CODE/cassandra/
cp $CODE/xml/build.properties.default $CODE/cassandra/

ant
```

##### Sample Output

If all goes well in the above step, you will see the following output
```
init:
maven-ant-tasks-localrepo:

maven-ant-tasks-download:

maven-ant-tasks-init:

maven-declare-dependencies:

_write-poms:

jar:
     [copy] Copying 1 file to /users/kannan11/RISE/cassandra/code/cassandra/build/classes/main/META-INF
     [copy] Copying 1 file to /users/kannan11/RISE/cassandra/code/cassandra/build/classes/thrift/META-INF
     [copy] Copying 1 file to /users/kannan11/RISE/cassandra/code/cassandra/build/classes/main/META-INF
     [copy] Copying 1 file to /users/kannan11/RISE/cassandra/code/cassandra/build/classes/thrift/META-INF
      [jar] Building jar: /users/kannan11/RISE/cassandra/code/cassandra/build/apache-cassandra-thrift-3.11.2-SNAPSHOT.jar
      [jar] Building jar: /users/kannan11/RISE/cassandra/code/cassandra/build/apache-cassandra-3.11.2-SNAPSHOT.jar
    [mkdir] Created dir: /users/kannan11/RISE/cassandra/code/cassandra/build/classes/stress/META-INF
    [mkdir] Created dir: /users/kannan11/RISE/cassandra/code/cassandra/build/tools/lib
      [jar] Building jar: /users/kannan11/RISE/cassandra/code/cassandra/build/tools/lib/stress.jar

BUILD SUCCESSFUL
Total time: 45 seconds
```



### Now we are going to install Cassandra's compression libraries. Cassandra compresses data when required
```
cd $CSRC
rm $CSRC/lib/snappy-java-1.1.1.7.jar
wget -O lib/snappy-java-1.1.2.6.jar https://repo1.maven.org/maven2/org/xerial/snappy/snappy-java/1.1.2.6/snappy-java-1.1.2.6.jar

//Build and replace JNA
git clone https://github.com/java-native-access/jna.git
cd jna
git checkout 4.2.2
ant //you will see an output that says bulild successful.

rm $CSRC/lib/jna-4.2.2.jar
cp $CSRC/jna/build/jna.jar $CSRC/lib/jna-4.2.2.jar

cd $CSRC

sudo rm -rf /usr/share/cassandra/*
sudo mkdir /usr/share/cassandra
sudo cp ./build/apache-cassandra-*.jar /usr/share/cassandra/
cp $CODE/cassandra.sh $CSRC/bin/
```

### Now we are going install a workload generator for Cassandra

```
cd $CODE
git clone https://gitlab.com/sudarsunkannan/mapkeeper
cp $CODE/xml/thrift/build.properties $CODE/mapkeeper/thrift-0.8.0/lib/java/build.properties
cd $CODE/mapkeeper/ycsb/YCSB
mvn clean package
```

##### Sample Output
If successfull, you will see messages similar to the below message
```
INFO] ------------------------------------------------------------------------
[INFO] Reactor Summary for YCSB Root 0.9.0-SNAPSHOT:
[INFO]
[INFO] YCSB Root .......................................... SUCCESS [ 10.471 s]
[INFO] Core YCSB .......................................... SUCCESS [ 11.536 s]
[INFO] Per Datastore Binding descriptor ................... SUCCESS [  0.202 s]
[INFO] YCSB Datastore Binding Parent ...................... SUCCESS [  1.781 s]
[INFO] Cassandra DB Binding ............................... SUCCESS [  4.579 s]
[INFO] Cassandra 2.1+ DB Binding .......................... SUCCESS [ 26.630 s]
[INFO] Mapkeeper DB Binding ............................... SUCCESS [  0.784 s]
[INFO] memcached binding .................................. SUCCESS [  0.819 s]
[INFO] YCSB Release Distribution Builder .................. SUCCESS [  1.757 s]
[INFO] ------------------------------------------------------------------------
[INFO] BUILD SUCCESS
[INFO] ------------------------------------------------------------------------
[INFO] Total time:  58.732 s
```


### Now, if all goes well, time to run Cassandra and the benchmark. Lets kill all pending java processes
```
sudo killall java
sleep 2
sudo killall java
sleep 2
```

### Time to launch Cassandra application. Note, it runs as a background server
```
cd $CSRC
//Delete current data folder and let us start from scratch
rm -rf $CSRC/data/*
 //here is where Cassandra stores the database
mkdir -p $CSRC/data/data

//Now, let's run Cassandra
$CSRC/bin/cassandra
sleep 5
```


##### Sample Output

If successful, you should see a message similar to this
```
...
INFO  [MigrationStage:1] 2020-07-07 22:59:36,383 ViewManager.java:137 - Not submitting build tasks for views in keyspace system_auth as storage service is not initialized
INFO  [MigrationStage:1] 2020-07-07 22:59:36,386 ColumnFamilyStore.java:411 - Initializing system_auth.resource_role_permissons_index
INFO  [MigrationStage:1] 2020-07-07 22:59:36,392 ColumnFamilyStore.java:411 - Initializing system_auth.role_members
INFO  [MigrationStage:1] 2020-07-07 22:59:36,397 ColumnFamilyStore.java:411 - Initializing system_auth.role_permissions
INFO  [MigrationStage:1] 2020-07-07 22:59:36,402 ColumnFamilyStore.java:411 - Initializing system_auth.roles
...
```

### Check if a JAVA process is running
```
ps -e | grep "java"

//You will see a java process with some process ID similar to this
...pts/0    00:00:28 java
```


### Now time to run the actual workload
```
cd $YCSBHOME
sleep 5

//Execute these commands to create ycsb keyspace with cassandra db
$CSRC/bin/cqlsh $HOST -e "create keyspace ycsb WITH REPLICATION = {'class' : 'SimpleStrategy', 'replication_factor': 1 }; USE ycsb; create table usertable (y_id varchar primary key, field0 varchar, field1 varchar, field2 varchar,field3 varchar,field4 varchar, field5 varchar, field6 varchar,field7 varchar,field8 varchar,field9 varchar);"

sleep 5

//First is the Warm up phase. Load the database

$YCSBHOME/bin/ycsb load cassandra2-cql -p hosts=$HOST -p port=$PORT -p recordcount=$OPSCNT -P $YCSBHOME/workloads/workloada -s
```

##### Sample Output

If all goes well, your warm-up output will look something like the below info
```
[OVERALL], RunTime(ms), 543.0
[OVERALL], Throughput(ops/sec), 0.0
[CLEANUP], Operations, 7.0
[CLEANUP], AverageLatency(us), 3.857142857142857
[CLEANUP], MinLatency(us), 2.0
[CLEANUP], MaxLatency(us), 8.0
[CLEANUP], 95thPercentileLatency(us), 8.0
[CLEANUP], 99thPercentileLatency(us), 8.0
[INSERT], Operations, 0.0
[INSERT], AverageLatency(us), NaN
[INSERT], MinLatency(us), 9.223372036854776E18
[INSERT], MaxLatency(us), 0.0
[INSERT], 95thPercentileLatency(us), 0.0
[INSERT], 99thPercentileLatency(us), 0.0
[INSERT], Return=ERROR, 7
[INSERT-FAILED], Operations, 7.0
[INSERT-FAILED], AverageLatency(us), 21861.714285714286
[INSERT-FAILED], MinLatency(us), 20768.0
[INSERT-FAILED], MaxLatency(us), 23183.0
[INSERT-FAILED], 95thPercentileLatency(us), 23183.0
[INSERT-FAILED], 99thPercentileLatency(us), 23183.0

```

### Check if a JAVA (Cassandra server) process is running
```
ps -e | grep "java"
```

##### Sample Output

```
...pts/0    00:00:28 java
```

If not, then re-run the server
```
$CSRC/bin/cassandra
```


```
// Now, we are going to run the benchmark
        
$YCSBHOME/bin/ycsb run cassandra2-cql -p hosts=$HOST -p port=$PORT -p recordcount=$OPSCNT -P $YCSBHOME/workloads/workloada
```

##### Sample Output

If successful, you will see the following output
```

[OVERALL], RunTime(ms), 8862.0
[OVERALL], Throughput(ops/sec), 11284.134506883322
[READ], Operations, 34885.0
[READ], AverageLatency(us), 302.29677511824565
[READ], MinLatency(us), 133.0
[READ], MaxLatency(us), 33791.0
[READ], 95thPercentileLatency(us), 537.0
[READ], 99thPercentileLatency(us), 916.0
[READ], Return=OK, 34885
[READ], Return=NOT_FOUND, 45220
[READ-MODIFY-WRITE], Operations, 40268.0
[READ-MODIFY-WRITE], AverageLatency(us), 651.916335551803
[READ-MODIFY-WRITE], MinLatency(us), 258.0
[READ-MODIFY-WRITE], MaxLatency(us), 73023.0
[READ-MODIFY-WRITE], 95thPercentileLatency(us), 1248.0
[READ-MODIFY-WRITE], 99thPercentileLatency(us), 2219.0
[CLEANUP], Operations, 8.0
[CLEANUP], AverageLatency(us), 279177.25
[CLEANUP], MinLatency(us), 2.0
[CLEANUP], MaxLatency(us), 2234367.0
[CLEANUP], 95thPercentileLatency(us), 2234367.0
[CLEANUP], 99thPercentileLatency(us), 2234367.0
[READ-FAILED], Operations, 45220.0
[READ-FAILED], AverageLatency(us), 330.9077620521893
[READ-FAILED], MinLatency(us), 112.0
[READ-FAILED], MaxLatency(us), 68031.0
[READ-FAILED], 95thPercentileLatency(us), 660.0
[READ-FAILED], 99thPercentileLatency(us), 1285.0
[UPDATE], Operations, 60163.0
[UPDATE], AverageLatency(us), 329.45192227781195
[UPDATE], MinLatency(us), 124.0
[UPDATE], MaxLatency(us), 68031.0
[UPDATE], 95thPercentileLatency(us), 630.0
[UPDATE], 99thPercentileLatency(us), 1155.0
[UPDATE], Return=OK, 60163
```


#### Running in Multiple node

1. Enable a ssh connection across nodes by adding every node's SSH key to every other node.

First, execute this in add the nodes
```
ssh-keygen
```

2. Add the keys to ~/.ssh/authorized_keys 
```
cat ~/.ssh/id_rsa.pub
```

Add the generated key to ~/.ssh/authorized_keys

3. Now, try to do the SSH between each node and make sure they can ssh.

4. Change the HOST information on each of the file

```
vim $CODE/scripts/setvars.sh
//Change the HOST variable 
export HOST=155.98.36.107 //IP address of the node
```

5. Set the environmental variables again

```
source $CODE/scripts/setvars.sh

```

6. Now, time to change host information

Open $CODE/cassandra/conf/cassandra.yaml file.

First change the listen_address and RPC address
```
listen_address: 155.98.36.111 //IP address of your machine
rpc_address: 155.98.36.111
```

Next, the seed info with all the nodes in the Cluster
```
- seeds: "155.98.36.111,155.98.36.107" //this should include all the nodes
```


7. Now time to run cassandra in each node

```
scripts/launch-cassandra.sh //must be done in all nodes
```

8. Check how my nodes are current running
```
$CODE/cassandra/bin/nodetool status
```

You will see some status like this

```
Picked up JAVA_TOOL_OPTIONS: -Dfile.encoding=UTF8
Datacenter: datacenter1
=======================
Status=Up/Down
|/ State=Normal/Leaving/Joining/Moving
--  Address        Load       Tokens       Owns (effective)  Host ID                               Rack
UN  155.98.36.107  103.69 KiB  256          100.0%            6125f211-6632-4b2a-8f71-10fda00a11c4  rack1
UN  155.98.36.111  118.07 KiB  256          100.0%            bd40b4d8-9802-4bdd-b532-d99e3625865d  rack1
```

9. Now time to run the benchmark
```
scripts/run.sh
```















