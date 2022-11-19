#!/bin/bash

set -x
set -e

if [ -z $NO_DEPS ]; then
    export HOMEBREW_NO_INSTALL_CLEANUP=1
    export HOMEBREW_NO_AUTO_UPDATE=1

    if ! command -v brew &> /dev/null
    then
        echo "brew could not be found in PATH, please install it, see: https://brew.sh"
        exit
    fi

    if [ -z $OPENMP_MULTI_ARCH ]; then
        brew install libomp
    else
        armloc=$(brew fetch --bottle-tag=arm64_big_sur libomp | grep -i downloaded | grep tar.gz | cut -f2 -d ":" | xargs echo)
        x64loc=$(brew fetch --bottle-tag=big_sur libomp | grep -i downloaded | grep tar.gz | cut -f2 -d ":" | xargs echo)
        cp $armloc /tmp/libomp-arm64.tar.gz
        mkdir /tmp/libomp-arm64 || true
        tar -xzvf /tmp/libomp-arm64.tar.gz -C /tmp/libomp-arm64
        cp $x64loc /tmp/libomp-x86_64.tar.gz
        mkdir /tmp/libomp-x86_64 || true
        tar -xzvf /tmp/libomp-x86_64.tar.gz -C /tmp/libomp-x86_64
    fi
    if [ -z $BOOST_FROM_SOURCE ]; then
        brew install boost
    fi
    if [ "$1" = "python3lib" ]; then
        if [ -z $PYTHON_EXECUTABLE ]; then
            brew install python3
        fi
        if [ -z $BOOST_FROM_SOURCE ]; then
            brew install boost-python3
        fi
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

if [ -n "$BOOST_FROM_SOURCE" ]; then
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
    ./b2 threading=multi \
        -j$(sysctl -n hw.logicalcpu) \
        variant="$2" \
        link=static \
        architecture=combined \
        address-model=${ADDRESS_MODEL:-"32_64"} \
        --layout=system \
        --build-type=minimal \
        ${GOT_USER_CONFIG:+"--user-config=./user-config.jam"} \
        cxxflags='-fPIC' \
        install \
        --prefix="${BOOST_PREFIX:-"$HOME/local"}"
fi