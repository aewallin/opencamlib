#!/bin/bash

set -x
set -e

if [ "$1" = "all" ]; then
    ./scripts/build-ubuntu.sh cxxlib debug $3
    ./scripts/build-ubuntu.sh cxxlib release $3
    ./scripts/build-ubuntu.sh python3lib debug $3
    ./scripts/build-ubuntu.sh python3lib release $3
    ./scripts/build-ubuntu.sh nodejslib debug $3
    ./scripts/build-ubuntu.sh nodejslib release $3
    ./scripts/build-ubuntu.sh emscriptenlib debug $3
    ./scripts/build-ubuntu.sh emscriptenlib release $3
    exit 0
fi

BUILD_DIR="build/${1}"
if [ "$2" = "release" ]; then
    BUILD_DIR="$BUILD_DIR/release"
    BUILD_TYPE="Release"
else
    BUILD_DIR="$BUILD_DIR/debug"
    BUILD_TYPE="Debug"
fi

if [ "$3" = "clean" ]; then
    echo $BUILD_DIR
    rm -rf $BUILD_DIR || true
fi

mkdir -p $BUILD_DIR || true

if [ -n "$BOOST_FROM_SOURCE" ] || [ "$1" = "emscriptenlib" ]; then
    BOOST_PREFIX=${BOOST_PREFIX:-"$HOME/local"}
fi

if [ "$1" = "cxxlib" ]; then
    cd $BUILD_DIR
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_CXX_LIB="ON" \
        ${BOOST_PREFIX:+"-D BOOST_ROOT=$BOOST_PREFIX"} \
        ../../..
    cmake --build . -j$(nproc)
    sudo cmake --install .
elif [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
    cd ../..
    if [ "$2" = "release" ]; then
        ./src/nodejslib/node_modules/.bin/cmake-js \
            build \
            --out "${BUILD_DIR}" \
            --CD BUILD_NODEJS_LIB="ON" \
            ${BOOST_PREFIX:+"--CD BOOST_ROOT=$BOOST_PREFIX"}
        mkdir -p src/npmpackage/build/Release || true
        cp "$BUILD_DIR/ocl.node" src/npmpackage/build/Release/ocl-$(node -e "console.log(process.platform)")-$(node -e "console.log(process.arch)").node || true
    else
        ./src/nodejslib/node_modules/.bin/cmake-js \
            build \
            --out "${BUILD_DIR}" \
            --CD BUILD_NODEJS_LIB="ON" \
            ${BOOST_PREFIX:+"--CD BOOST_ROOT=$BOOST_PREFIX"} \
            --debug
        mkdir -p src/npmpackage/build/Debug || true
        cp "$BUILD_DIR/ocl.node" src/npmpackage/build/Debug/ocl-$(node -e "console.log(process.platform)")-$(node -e "console.log(process.arch)").node || true
    fi
elif [ "$1" = "python3lib" ]; then
    cd $BUILD_DIR
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_PY_LIB="ON" \
        ${BOOST_PREFIX:+"-D BOOST_ROOT=$BOOST_PREFIX"} \
        ${PYTHON_PREFIX:+"-D Python3_ROOT_DIR=$PYTHON_PREFIX"} \
        ../../..
    cmake --build . -j$(nproc)
    sudo cmake --install .
elif [ "$1" = "emscriptenlib" ]; then
    source ../emsdk/emsdk_env.sh
    cd $BUILD_DIR
    emcmake cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF" \
        ${BOOST_PREFIX:+"-D BOOST_ROOT=$BOOST_PREFIX"} \
        ../../..
    emmake make -j$(nproc)
    cd ../../..
    mkdir -p "src/npmpackage/build" || true
    cp "$BUILD_DIR/ocl.js" src/npmpackage/build/ || true
else
    echo "Usage: ./scripts/build-macos.sh lib build_type [clean]"
    echo "  lib: one of cxxlib, nodejslib, python3lib, emscriptenlib"
    echo "  build_type: one of debug, release"
    echo "  clean: optional, removes the build directory before building"
fi

