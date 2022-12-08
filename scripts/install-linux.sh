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
        --help|--*)
            print_help ;;
        *)
            POSITIONAL_ARGS+=("$1") ;;
    esac
    shift
done
set -- "${POSITIONAL_ARGS[@]}"

if [ -z $OCL_NO_DEPS ]; then
    sudo apt update
    sudo apt install -y --no-install-recommends git cmake build-essential
    if [ -z $OCL_BOOST_FROM_SOURCE ]; then
        sudo apt install -y libboost-dev
    fi

    if [ "$1" = "python3lib" ]; then
        if [ -z $OCL_PYTHON_EXECUTABLE ]; then
            sudo apt install -y --no-install-recommends python3
        fi
        if [ -z $OCL_BOOST_FROM_SOURCE ]; then
            sudo apt install -y --no-install-recommends libboost-python-dev
        fi
    fi

    if [ "$1" = "nodejslib" ]; then
        sudo apt install -y --no-install-recommends nodejs npm
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
    if [ -z $OCL_BOOST_HEADERS_ONLY ]; then
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
        ./bootstrap.sh
        ./b2 \
            -a \
            threading=multi \
            -j$(nproc) \
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