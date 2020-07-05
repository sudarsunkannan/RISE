#!/bin/bash


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
    if [ $(mount | grep -c $SSD_PARTITION) == 1 ]
    then
        echo "Already mounted"
    else
        sudo fdisk $SSD_DEVICE
        sudo mkfs.ext4 $SSD_PARTITION
        sudo mount $SSD_PARTITION $SSD
    fi
}

COPY_TO_SSD() {

 cp -r $HOME/butterflyeffect $SSD/

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
sudo pip install psutil
}

mkdir $CODE
cd $CODE
#INSTALL_SYSTEM_LIBS
FORMAT_SSD
COPY_TO_SSD
cd $SSD
#INSTALL_PERF_TOOLS
