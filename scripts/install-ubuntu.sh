#!/bin/sh -xe

sudo apt-get update

sudo apt-get install -y python3 python3-dev

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
fi

# we have to compile boost from source because the official packages did not compile with the "-fPIC" flag
mkdir /tmp/boost
cd /tmp/boost
wget -nv -O boost.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
tar -zxf boost.tar.gz

cd boost_1_80_0
if [ "$1" = "python3lib" ]; then
    sudo ./bootstrap.sh --prefix=/usr/local
    if [ -n "${PYTHON_EXECUTABLE}" ]; then
        PYTHON_VERSION=`${PYTHON_EXECUTABLE} -c 'import sys; version=sys.version_info[:3]; print("{0}.{1}".format(*version))'`
        echo "using python : ${PYTHON_VERSION} : ${PYTHON_EXECUTABLE} ;" > ./project-config.jam
    fi
else
    sudo ./bootstrap.sh --prefix=/usr/local
fi

if [ "$1" = "python3lib" ]; then
    sudo ./b2 address-model=64 threading=multi --with-python -j2 variant=$2 link=static --layout=system cxxflags='-fPIC' install
else
    sudo ./b2 address-model=64 threading=multi -j2 variant=$2 link=static --layout=system cxxflags='-fPIC' install
fi