#!/bin/bash

cp $CODE/xml/*.xml $CODE/cassandra/
cp $CODE/xml/build.properties.default $CODE/cassandra/

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

INSTALL_PERF_TOOLS() {
  cd $CODE 
  if [ ! -d "perf-tools" ]; then
      git clone https://github.com/brendangregg/perf-tools.git  
  fi
}

FORMAT_SSD() {
    mkdir $SSD
    sudo mount $SSD_PARTITION $SSD
    sudo chown -R $USER $SSD
    if [ $? -eq 0 ]; then
        echo OK
    else
        sudo fdisk $SSD_DEVICE
        sudo mkfs.ext4 $SSD_PARTITION
        sudo mount $SSD_PARTITION $SSD
    fi
}

INSTALL_YCSB() {
    cd $CODE
    if [ ! -d "mapkeeper" ]; then
        git clone https://gitlab.com/sudarsunkannan/mapkeeper
    fi
    cp $CODE/xml/thrift/build.properties $CODE/mapkeeper/thrift-0.8.0/lib/java/build.properties	
    cd $CODE/mapkeeper/ycsb/YCSB
    mvn clean package
}


INSTALL_CASANDARA_BINARY(){

    mkdir $CODE/cassandra	
    #echo "deb http://www.apache.org/dist/cassandra/debian 311x main" | sudo tee -a /etc/apt/sources.list.d/cassandra.sources.list
    #echo "deb-src http://www.apache.org/dist/cassandra/debian 311x main" | sudo tee -a /etc/apt/sources.list.d/cassandra.sources.list

    #gpg --keyserver pgp.mit.edu --recv-keys F758CE318D77295D
    #gpg --export --armor F758CE318D77295D | sudo apt-key add -

    #gpg --keyserver pgp.mit.edu --recv-keys 2B5C1B00
    #gpg --export --armor 2B5C1B00 | sudo apt-key add -

    #gpg --keyserver pgp.mit.edu --recv-keys 0353B12C
    #gpg --export --armor 0353B12C | sudo apt-key add -
    sudo apt-key adv --keyserver pool.sks-keyservers.net --recv-key A278B781FE4B2BDA

    sudo apt-get update
    sudo apt-get install -y --force-yes cassandra
    #RUN_YCSB_CASSANDARA
}


CASANDARA_INSTALL_JAVA(){
   sudo add-apt-repository ppa:openjdk-r/ppa -y
   sudo apt-get update
   sudo apt-get install openjdk-8-jdk -y
   echo "Now Set the default JAVA version to Open JDK 7"
   sudo update-alternatives --config java
   java -version
}

CASANDARA_SET_ENV() {

    export LANG="en_US.UTF-8"
    export JAVA_TOOL_OPTIONS="-Dfile.encoding=UTF8"
    export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
    export ANT_OPTS="-Xms4G -Xmx4G"
    export ANT_HOME=$CODE/apache-ant-1.10.0-bin
    export PATH=$PATH:$ANT_HOME/bin
}

DOWNLOAD_CASANDARA_SOURCE(){

    #wget http://archive.apache.org/dist/cassandra/3.9/apache-cassandra-3.9-src.tar.gz
    #tar -xvzf apache-cassandra-3.9-src.tar.gz
    git clone https://github.com/apache/cassandra.git

}

INSTALL_ANT() {
    cd $CODE
    sudo apt-get update
    sudo apt-get install -y git tar g++ make automake autoconf libtool  wget patch libx11-dev libxt-dev pkg-config texinfo locales-all unzip python
    sudo apt-get install -y maven
    #sudo apt-get install -y openjdk-7-jre openjdk-7-jdk  #(If other JDK installed, then why install this?)
    wget http://archive.apache.org/dist/ant/binaries/apache-ant-1.10.0-bin.tar.gz
    tar -xvf apache-ant-1.10.0-bin.tar.gz
}


INSTALL_CASANDARA_SOURCE_3.9(){

    #if [ ! -d "/usr/share/cassandra" ]; then
    INSTALL_CASANDARA_BINARY
    #fi
    #CASANDARA_INSTALL_JAVA


    if [ ! -d "apache-cassandra-3.9-src" ]; then
        DOWNLOAD_CASANDARA_SOURCE
    fi	


    cd apache-cassandra-3.9*

    #keep a backup if installed version exists and no backup exists
    if [ ! -d "/usr/share/cassandra-orig" ]; then
        sudo cp -rf  /usr/share/cassandra  /usr/share/cassandra-orig
    fi
    sudo cp ./build/apache-cassandra-3.9-SNAPSHOT.jar /usr/share/cassandra/apache-cassandra-3.9.jar
    sudo cp ./build/apache-cassandra-thrift-3.9-SNAPSHOT.jar /usr/share/cassandra/apache-cassandra-thrift-3.9.jar

}


INSTALL_CASANDARA_SOURCE(){

    cd $CODE
    INSTALL_CASANDARA_BINARY  


   # if [ ! -d "apache-ant-1.10.0-bin" ]; then
    INSTALL_ANT
    #fi	

    cp $CODE/xml/libraries.properties $CODE/apache-ant-1.10.0/lib/libraries.properties	

    #if [ ! -d "cassandra" ]; then
    DOWNLOAD_CASANDARA_SOURCE
    #fi

    cd cassandra
    git checkout cassandra-3.11.2
    sed  -i 's/Xss256k/Xss32m/g' build.xml conf/jvm.options 

    sudo apt-get install ant
    ant
	
    #Step 2: Replace x86 specific jar files
    cd $CSRC 
    rm $CSRC/lib/snappy-java-1.1.1.7.jar
    wget -O lib/snappy-java-1.1.2.6.jar https://repo1.maven.org/maven2/org/xerial/snappy/snappy-java/1.1.2.6/snappy-java-1.1.2.6.jar 

    #Build and replace JNA
    git clone https://github.com/java-native-access/jna.git
    cd jna
    git checkout 4.2.2
    ant

    rm $CSRC/lib/jna-4.2.2.jar
    cp $CSRC/jna/build/jna.jar $CSRC/lib/jna-4.2.2.jar

    cd $CSRC 
    #ant test

    sudo rm -rf /usr/share/cassandra/*
    sudo mkdir /usr/share/cassandra
    sudo cp ./build/apache-cassandra-*.jar /usr/share/cassandra/
    cp $CODE/cassandra.sh $CSRC/bin/
}


RUN_CASSANDARA() {
    #$YCSBHOME/cassandra/start_sevice.sh
    cd $CSRC

    #Delete data folder
    mkdir $SHARED_DATA
    #rm -rf $SHARED_DATA/*
    rm -rf $CSRC/data/*
    mkdir -p $CSRC/data/data
    $CSRC/bin/cassandra
    #/usr/sbin/cassandra
    #/usr/sbin/cassandra "--preferred=1"
    sleep 5
}



RUN_YCSB_CASSANDARA() {

    INSTALL_CASANDARA_SOURCE

    #cd $YCSBHOME/cassandra
    #./start_sevice.sh 
    RUN_CASSANDARA
    sleep 5
}

INSTALL_JAVA() {
    sudo add-apt-repository ppa:webupd8team/java
    sudo apt-get update
    sudo apt-get install -y oracle-java8-set-default
    java -version
}

INSTALL_CMAKE(){
    cd $CODE
    wget https://cmake.org/files/v3.7/cmake-3.7.0-rc3.tar.gz
    tar zxvf cmake-3.7.0-rc3.tar.gz
    cd cmake-3.7.0*
    ./configure
    ./bootstrap
    make -j16
    make install
}

INSTALL_SYSTEM_LIBS(){
sudo apt-get install -y git
#git commit --amend --reset-author

sudo apt-get install -y software-properties-common
sudo apt-get install -y python3-software-properties
sudo apt-get install -y python-software-properties
sudo apt-get install -y unzip
sudo apt-get install -y python-setuptools python-dev build-essential
sudo easy_install pip
sudo apt-get install -y numactl
sudo apt-get install -y libsqlite3-dev
sudo apt-get install -y libnuma-dev
sudo apt-get install -y libkrb5-dev
sudo apt-get install -y libsasl2-dev
sudo apt-get install -y cmake
sudo apt-get install -y build-essential
sudo apt-get install -y maven
sudo apt-get install -y fio
sudo apt-get install -y libbfio-dev
sudo apt-get install -y libboost-dev
sudo apt-get install -y libboost-thread-dev
sudo apt-get install -y libboost-system-dev
sudo apt-get install -y libboost-program-options-dev
sudo apt-get install -y libconfig-dev
sudo apt-get install -y uthash-dev
sudo apt-get install -y libmpich2-dev
sudo apt-get install -y libglib2.0-dev
sudo apt-get install -y cscope
sudo apt-get install -y msr-tools
sudo apt-get install -y msrtool
sudo apt-get install -y textinfo
sudo apt-get install -y dpkg
sudo pip install psutil
}

DESTROY

mkdir $CODE
cd $CODE
#fuser -k $PORT/tcp
#INSTALL_SYSTEM_LIBS
#CASANDARA_INSTALL_JAVA
INSTALL_CASANDARA_SOURCE
INSTALL_YCSB
DESTROY

#INSTALL_PERF_TOOLS

#FORMAT_SSD
#Install ycsb and casandara
#RUN_YCSB_CASSANDARA
#INSTALL_YCSB

