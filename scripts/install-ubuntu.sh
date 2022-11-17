#!/bin/bash

set -x
set -e

sudo apt update
sudo apt install -y git cmake build-essential doxygen texlive-latex-base
if [ -z $BOOST_FROM_SOURCE ]; then
    sudo apt install -y libboost-dev
fi

if [ "$1" = "python3lib" ]; then
    if [ -z $PYTHON_EXECUTABLE ]; then
        sudo apt install -y python3
    fi
    if [ -z $BOOST_FROM_SOURCE ]; then
        sudo apt install -y libboost-python-dev
    fi
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
fi

if [ -n "$BOOST_FROM_SOURCE" ] || [ "$1" = "emscriptenlib" ]; then
    rm -rf /tmp/boost || true
    mkdir /tmp/boost
    cd /tmp
    if [ ! -f boost.tar.gz ]; then
        wget -nv -O boost.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
    fi
    tar -zxf boost.tar.gz -C /tmp/boost
    cd boost/boost_1_80_0
    if [ "$1" = "python3lib" ]; then
        if [ -n "${PYTHON_EXECUTABLE}" ]; then
            PYTHON_VERSION=`${PYTHON_EXECUTABLE} -c 'import sys; version=sys.version_info[:3]; print("{0}.{1}".format(*version))'`
            echo "using python : ${PYTHON_VERSION} : ${PYTHON_EXECUTABLE} ;" > ./user-config.jam
        else
            echo "using python ;" > ./user-config.jam
        fi
        cat ./user-config.jam
        GOT_USER_CONFIG="1"
    fi
    ./bootstrap.sh
    ./b2 address-model=64 \
        threading=multi \
        -j$(nproc) \
        variant="$2" \
        link=static \
        --layout=system \
        --build-type=minimal \
        ${GOT_USER_CONFIG:+"--user-config=./user-config.jam"} \
        cxxflags='-fPIC' \
        install \
        --prefix="${BOOST_PREFIX:-"$HOME/local"}"
fi