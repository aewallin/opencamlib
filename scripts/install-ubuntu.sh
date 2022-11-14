#!/bin/sh -xe

sudo apt-get update

if [ "$1" = "python3lib" ]; then
    sudo apt-get install -y git cmake build-essential doxygen texlive-latex-base python3 python3-dev libboost-dev libboost-python-dev
else
    sudo apt-get install -y git cmake build-essential doxygen texlive-latex-base libboost-dev
fi

if [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
fi

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest

    # we will compile boost from source because find_package(Boost) is not working with emscripten
    # even with CMAKE_FIND_ROOT_PATH_MODE_INCLUDE=BOTH, CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=BOTH
    # and by setting CMAKE_FIND_ROOT_PATH=/ the library is not found, will investigate this later
    # and compile from source for now
    mkdir /tmp/boost
    cd /tmp/boost
    wget -nv -O boost.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
    tar -zxf boost.tar.gz

    cd boost_1_80_0
    sudo ./bootstrap.sh --prefix=/usr/local
    sudo ./b2 address-model=64 threading=multi -j2 variant=$2 link=static --layout=system cxxflags='-fPIC' install
fi
