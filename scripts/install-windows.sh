#!/bin/bash

set -x
set -e

choco install wget --no-progress
if [ -z $BOOST_FROM_SOURCE ]; then
    choco install boost-msvc-14.3 --no-progress
fi

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
fi

if [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
    cd ../..
fi

if [ -n "$BOOST_FROM_SOURCE" ] || [ "$1" = "emscriptenlib" ]; then
    rm -rf /c/boost || true
    mkdir /c/boost
    cd /c
    if [ ! -f boost.tar.gz ]; then
        wget -nv -O boost.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
    fi
    tar -zxf boost.tar.gz -C /c/boost
    cd /c/boost/boost_1_80_0
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
    ./bootstrap.bat
    ./b2 address-model=64 \
        threading=multi \
        -j2 \
        variant="$2" \
        link=static \
        --layout=system \
        --build-type=minimal \
        ${GOT_USER_CONFIG:+"--user-config=./user-config.jam"} \
        install \
        --prefix="${BOOST_PREFIX:-"/c/boost"}"
fi
