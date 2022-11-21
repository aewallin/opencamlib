#!/bin/bash

set -x
set -e

print_help() {
    cat "$(dirname $(readlink -f $0))/usage-build.txt"
    exit 1
}

POSITIONAL_ARGS=()
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --install-prefix) OCL_INSTALL_PREFIX="$2"; shift ;;
        --boost-prefix) OCL_BOOST_PREFIX="$2"; shift ;;
        --python-prefix) OCL_PYTHON_PREFIX="$2"; shift ;;
        --sudo-install) OCL_SUDO_INSTALL="1"; ;;
        --clean) OCL_CLEAN="1"; ;;
        --no-install) OCL_NO_INSTALL="1"; ;;
        --help|--*)
            print_help ;;
        *)
            POSITIONAL_ARGS+=("$1") ;;
    esac
    shift
done
set -- "${POSITIONAL_ARGS[@]}"

if [ -n "$OCL_BOOST_FROM_SOURCE" ]; then
    OCL_BOOST_PREFIX=${OCL_BOOST_PREFIX:-"$HOME/local"}
fi

BUILD_DIR="build/${1}"
if [ "$2" = "debug" ]; then
    BUILD_DIR="$BUILD_DIR/debug"
    BUILD_TYPE="Debug"
else
    BUILD_DIR="$BUILD_DIR/release"
    BUILD_TYPE="Release"
fi

if [ "$OCL_CLEAN" = "1" ]; then
    echo $BUILD_DIR
    rm -rf $BUILD_DIR || true
fi
mkdir -p $BUILD_DIR || true
cd $BUILD_DIR

NUM_PROCS=$(nproc)

if [ "$1" = "cxxlib" ]; then
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_CXX_LIB="ON" \
        ${OCL_INSTALL_PREFIX:+"-DCMAKE_INSTALL_PREFIX=${OCL_INSTALL_PREFIX}"} \
        ${OCL_BOOST_PREFIX:+"-DBOOST_ROOT=${OCL_BOOST_PREFIX}"} \
        ../../..
    cmake --build . -j$NUM_PROCS
elif [ "$1" = "nodejslib" ]; then
    ../../../src/nodejslib/node_modules/.bin/cmake-js \
        build \
        --directory "../../.." \
        --out "." \
        --parallel $NUM_PROCS \
        --CD BUILD_NODEJS_LIB="ON" \
        ${OCL_INSTALL_PREFIX:+"--CDCMAKE_INSTALL_PREFIX=${OCL_INSTALL_PREFIX}"} \
        ${OCL_BOOST_PREFIX:+"--CDBoost_ROOT=${OCL_BOOST_PREFIX}"} \
        --config "${BUILD_TYPE}"
elif [ "$1" = "python3lib" ]; then
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_PY_LIB="ON" \
        ${OCL_INSTALL_PREFIX:+"-DCMAKE_INSTALL_PREFIX=${OCL_INSTALL_PREFIX}"} \
        ${OCL_PYTHON_PREFIX:+"-DPython3_ROOT_DIR=${OCL_PYTHON_PREFIX}"} \
        ${OCL_BOOST_PREFIX:+"-DBoost_ROOT=${OCL_BOOST_PREFIX}"} \
        ../../..
    cmake --build . -j$NUM_PROCS
elif [ "$1" = "emscriptenlib" ]; then
    source ../../../../emsdk/emsdk_env.sh
    emcmake cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF" \
        ${OCL_INSTALL_PREFIX:+"-DCMAKE_INSTALL_PREFIX=${OCL_INSTALL_PREFIX}"} \
        ${OCL_BOOST_PREFIX:+"-DBoost_ROOT=${OCL_BOOST_PREFIX}"} \
        ../../..
    emmake make -j$NUM_PROCS
else
    print_help
fi

if [ -z $OCL_NO_INSTALL ]; then
    if [ "$OCL_SUDO_INSTALL" = "1" ]; then
        sudo cmake --install .
    else
        cmake --install .
    fi
fi