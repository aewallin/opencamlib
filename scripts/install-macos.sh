#!/bin/bash

set -x
set -e

PROJECT_DIR="$(pwd)"

print_help() {
    cat "$(dirname $(readlink -f $0))/usage-install.txt"
    exit 1
}

POSITIONAL_ARGS=()
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --boost-prefix) OCL_BOOST_PREFIX="$2"; shift ;;
        --boost-from-source) OCL_BOOST_FROM_SOURCE="1"; ;;
        --boost-address-model) OCL_BOOST_ADDRESS_MODEL="$2"; shift ;;
        --boost-architecture) OCL_BOOST_ARCHITECTURE="$2"; shift ;;
        --boost-headers-only) OCL_BOOST_HEADERS_ONLY="1"; ;;
        --python-executable) OCL_PYTHON_EXECUTABLE="$2"; shift ;;
        --no-deps) OCL_NO_DEPS="1"; ;;
        --download-openmp-multi-arch) OCL_OPENMP_MULTI_ARCH="1"; ;;
        --help|--*)
            print_help ;;
        *)
            POSITIONAL_ARGS+=("$1") ;;
    esac
    shift
done
set -- "${POSITIONAL_ARGS[@]}"

if [ -z $OCL_NO_DEPS ]; then
    export HOMEBREW_NO_INSTALL_CLEANUP=1
    export HOMEBREW_NO_AUTO_UPDATE=1

    if ! command -v brew &> /dev/null
    then
        echo "brew could not be found in PATH, please install it, see: https://brew.sh"
        exit
    fi

    if [ -z $OCL_OPENMP_MULTI_ARCH ]; then
        brew install libomp
    else
        armloc=$(brew fetch --bottle-tag=arm64_big_sur libomp | grep -i downloaded | grep tar.gz | cut -f2 -d ":" | xargs echo)
        x64loc=$(brew fetch --bottle-tag=big_sur libomp | grep -i downloaded | grep tar.gz | cut -f2 -d ":" | xargs echo)
        cp $armloc /tmp/libomp-arm64.tar.gz
        mkdir /tmp/libomp-arm64 || true
        tar -xzvf /tmp/libomp-arm64.tar.gz -C /tmp/libomp-arm64
        LIBOMP_PREFIX=$(find /tmp/libomp-arm64/libomp -depth 1 | head -1)
        mv "${LIBOMP_PREFIX}" "/tmp/libomp-arm64/libomp/fixed"

        cp $x64loc /tmp/libomp-x86_64.tar.gz
        mkdir /tmp/libomp-x86_64 || true
        tar -xzvf /tmp/libomp-x86_64.tar.gz -C /tmp/libomp-x86_64
        LIBOMP_PREFIX=$(find /tmp/libomp-x86_64/libomp -depth 1 | head -1)
        mv "${LIBOMP_PREFIX}" "/tmp/libomp-x86_64/libomp/fixed"
    fi
    if [ -z $OCL_BOOST_FROM_SOURCE ]; then
        brew install boost
    fi
    if [ "$1" = "python3lib" ]; then
        if [ -z $OCL_PYTHON_EXECUTABLE ]; then
            brew install python@3.11
        fi
        if [ -z $OCL_BOOST_FROM_SOURCE ]; then
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

if [ -n "$OCL_BOOST_FROM_SOURCE" ]; then
    rm -rf /tmp/boost || true
    mkdir /tmp/boost
    cd /tmp
    if [ ! -f boost_1_80_0.tar.gz ]; then
        wget -nv -O boost_1_80_0.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
    fi
    tar -zxf boost_1_80_0.tar.gz -C /tmp/boost
    cd boost/boost_1_80_0/libs/python
    git apply "${PROJECT_DIR}/.github/patches/boost-python-3.11.patch"
    cd ../..
    if [ "$1" = "python3lib" ]; then
        if [ -n "${OCL_PYTHON_EXECUTABLE}" ]; then
            PYTHON_VERSION=`${OCL_PYTHON_EXECUTABLE} -c 'import sys; version=sys.version_info[:3]; print("{0}.{1}".format(*version))'`
            echo "using python : ${PYTHON_VERSION} : ${OCL_PYTHON_EXECUTABLE} ;" > ./user-config.jam
        else
            echo "using python ;" > ./user-config.jam
        fi
        cat ./user-config.jam
        GOT_USER_CONFIG="1"
    fi
    if [ -z $OCL_BOOST_HEADERS_ONLY ]; then
        ./bootstrap.sh
        ./b2 \
            -a \
            threading=multi \
            -j$(sysctl -n hw.logicalcpu) \
            variant="$2" \
            link=static \
            ${OCL_BOOST_ADDRESS_MODEL:+"address-model=${OCL_BOOST_ADDRESS_MODEL}"} \
            ${OCL_BOOST_ARCHITECTURE:+"architecture=${OCL_BOOST_ARCHITECTURE}"} \
            --layout=system \
            --with-python \
            ${GOT_USER_CONFIG:+"--user-config=./user-config.jam"} \
            cxxflags='-fPIC' \
            stage
    fi
fi