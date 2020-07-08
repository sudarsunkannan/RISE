
###### Set environmental variable
```
cd RISE/cassandra/code
source scripts/setvars.sh
```

###### Set copy required XML files

```
cp $CODE/xml/*.xml $CODE/cassandra/
cp $CODE/xml/build.properties.default $CODE/cassandra/
```

###### Install system libraries
```
$CODE/scripts/install_libs.sh
```

###### Install JAVA 8

```
sudo add-apt-repository ppa:openjdk-r/ppa -y
sudo apt-get update
sudo apt-get install openjdk-8-jdk -y
echo "Now Set the default JAVA version to Open JDK 8"
sudo update-alternatives --config java
java -version
```

###### Install CASSANDRA binary
```
cd $CODE	
mkdir $CODE/cassandra
sudo apt-key adv --keyserver pool.sks-keyservers.net --recv-key A278B781FE4B2BDA
sudo apt-get update
sudo apt-get install -y --force-yes cassandra
```

###### Install Compiler for CASSANDRA source
```
sudo apt-get update
sudo apt-get install -y git tar g++ make automake autoconf libtool  wget patch libx11-dev libxt-dev pkg-config texinfo locales-all unzip python
sudo apt-get install -y maven
wget http://archive.apache.org/dist/ant/binaries/apache-ant-1.10.0-bin.tar.gz
tar -xvf apache-ant-1.10.0-bin.tar.gz
sudo apt-get install ant
cp $CODE/xml/libraries.properties $CODE/apache-ant-1.10.0/lib/libraries.properties
```

###### Now we are going to compile Cassandra from source 

```
cd $CODE
git clone https://github.com/apache/cassandra.git
cd cassandra
git checkout cassandra-3.11.2 
sed  -i 's/Xss256k/Xss32m/g' build.xml conf/jvm.options
ant


cd $CSRC
rm $CSRC/lib/snappy-java-1.1.1.7.jar
wget -O lib/snappy-java-1.1.2.6.jar https://repo1.maven.org/maven2/org/xerial/snappy/snappy-java/1.1.2.6/snappy-java-1.1.2.6.jar

//Build and replace JNA
git clone https://github.com/java-native-access/jna.git
cd jna
git checkout 4.2.2
ant

rm $CSRC/lib/jna-4.2.2.jar
cp $CSRC/jna/build/jna.jar $CSRC/lib/jna-4.2.2.jar

cd $CSRC

sudo rm -rf /usr/share/cassandra/*
sudo mkdir /usr/share/cassandra
sudo cp ./build/apache-cassandra-*.jar /usr/share/cassandra/
cp $CODE/cassandra.sh $CSRC/bin/
```

###### Now we are going install a workload generator for Cassandra

```
cd $CODE
if [ ! -d "mapkeeper" ]; then
git clone https://gitlab.com/sudarsunkannan/mapkeeper
fi
cp $CODE/xml/thrift/build.properties $CODE/mapkeeper/thrift-0.8.0/lib/java/build.properties
cd $CODE/mapkeeper/ycsb/YCSB
mvn clean package
```

###### Now, if all goes well, time to run Cassandra and the benchmark. Lets kill all pending java processes
```
sudo killall java
sleep 2
sudo killall java
sleep 2
```

###### Time to launch Cassandra application. Note, it runs as a background server
```
cd $CSRC
//Delete current data folder and let us start from scratch
mkdir $SHARED_DATA
rm -rf $CSRC/data/*
mkdir -p $CSRC/data/data
$CSRC/bin/cassandra
sleep 5
```

###### Check if a JAVA process is running
```
ps -e | grep "java"

//You will see a java process with an active PID
```


###### Now time to run the actual workload

```
cd $YCSBHOME
sleep 5

//Execute these commands to create ycsb keyspace with cassandra db
$CSRC/bin/cqlsh $HOST -e "create keyspace ycsb WITH REPLICATION = {'class' : 'SimpleStrategy', 'replication_factor': 1 }; USE ycsb; create table usertable (y_id varchar primary key, field0 varchar, field1 varchar, field2 varchar,field3 varchar,field4 varchar, field5 varchar, field6 varchar,field7 varchar,field8 varchar,field9 varchar);" #> ~/output

sleep 5

//First is the Warm up phase. Load the database

$YCSBHOME/bin/ycsb load cassandra2-cql -p hosts=$HOST -p port=$PORT -p recordcount=$OPSCNT -P $YCSBHOME/workloads/workloada -s
sleep 5

// Now, we are going to run the benchmark
        
$YCSBHOME/bin/ycsb run cassandra2-cql -p hosts=$HOST -p port=$PORT -p recordcount=$OPSCNT -P $YCSBHOME/workloads/workloada
sudo service cassandra stop
```







