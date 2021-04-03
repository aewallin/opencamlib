#!/bin/sh -xe

sudo apt-get update

if [ "$1" = "python2lib" ]; then
    if [ "$3" = "16.04" ] || [ "$3" == "18.04" ]; then
        sudo apt-get install -y python python-dev
    else
        sudo apt-get install -y python2 python2-dev
    fi
elif [ "$1" = "python3lib" ]; then
    sudo apt-get install -y python3 python3-dev
fi

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
fi

# find /usr/include -name pyconfig.h

# we have to compile boost from source because the official packages did not compile with the "-fPIC" flag
mkdir /tmp/boost
cd /tmp/boost
wget -nv -O boost.tar.gz https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.gz
tar -zxf boost.tar.gz

cd boost_1_75_0
if [ "$1" = "python3lib" ]; then
    sudo ./bootstrap.sh --with-python-version=3.6 --prefix=/usr/local
elif [ "$1" = "python2lib" ]; then
    sudo ./bootstrap.sh --with-python-version=2.7 --prefix=/usr/local
else
    sudo ./bootstrap.sh --prefix=/usr/local
fi

if [ "$1" = "python3lib" ] || [ "$1" = "python2lib" ]; then
    sudo ./b2 address-model=64 threading=multi --with-python -j2 variant=$2 link=static --layout=system cxxflags='-fPIC' install
else
    sudo ./b2 address-model=64 threading=multi -j2 variant=$2 link=static --layout=system cxxflags='-fPIC' install
fi