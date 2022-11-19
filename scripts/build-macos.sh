#!/bin/bash

set -x
set -e

if [ "$1" = "all" ]; then
    if [ "$2" = "debug" ] || [ -z $2 ]; then
        ./scripts/build-macos.sh cxxlib debug $3
        ./scripts/build-macos.sh python3lib debug $3
        ./scripts/build-macos.sh nodejslib debug $3
        ./scripts/build-macos.sh emscriptenlib debug $3
    elif [ "$2" = "release" ] || [ -z $2 ]; then
        ./scripts/build-macos.sh cxxlib release $3
        ./scripts/build-macos.sh python3lib release $3
        ./scripts/build-macos.sh nodejslib release $3
        ./scripts/build-macos.sh emscriptenlib release $3
    fi
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

if [ -n "$BOOST_FROM_SOURCE" ]; then
    BOOST_PREFIX=${BOOST_PREFIX:-"$HOME/local"}
fi

if [ "$1" = "cxxlib" ]; then
    cd $BUILD_DIR
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_CXX_LIB="ON" \
        ${BOOST_PREFIX:+"-D Boost_ROOT=$BOOST_PREFIX"} \
        ../../..
    cmake --build . -j$(sysctl -n hw.logicalcpu)
    if [ -z $NO_INSTALL ]; then
        cmake --install .
    fi
elif [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
    cd ../..
    if [ "$2" = "release" ]; then
        ./src/nodejslib/node_modules/.bin/cmake-js build \
            --out "${BUILD_DIR}" \
            --parallel $(sysctl -n hw.logicalcpu) \
            --CD BUILD_NODEJS_LIB="ON" \
            ${BOOST_PREFIX:+"--CD Boost_ROOT=$BOOST_PREFIX"}
        if [ -z $NO_INSTALL ]; then
            mkdir -p src/npmpackage/build/Release || true
            cp "$BUILD_DIR/ocl.node" src/npmpackage/build/Release/ocl-$(node -e "console.log(process.platform)")-$(node -e "console.log(process.arch)").node || true
        fi
    else
        ./src/nodejslib/node_modules/.bin/cmake-js build \
            --out "${BUILD_DIR}" \
            --parallel $(sysctl -n hw.logicalcpu) \
            --CD BUILD_NODEJS_LIB="ON" \
            ${BOOST_PREFIX:+"--CD Boost_ROOT=$BOOST_PREFIX"} \
            --debug
        if [ -z $NO_INSTALL ]; then
            mkdir -p src/npmpackage/build/Debug || true
            cp "$BUILD_DIR/ocl.node" src/npmpackage/build/Debug/ocl-$(node -e "console.log(process.platform)")-$(node -e "console.log(process.arch)").node || true
        fi
    fi
elif [ "$1" = "python3lib" ]; then
    cd $BUILD_DIR
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_PY_LIB="ON" \
        ${PYTHON_PREFIX:+"-D Python3_ROOT_DIR=$PYTHON_PREFIX"} \
        ${BOOST_PREFIX:+"-D Boost_ROOT=$BOOST_PREFIX"} \
        ../../..
    cmake --build . -j$(sysctl -n hw.logicalcpu)
    if [ -z $NO_INSTALL ]; then
        cmake --install .
    fi
elif [ "$1" = "emscriptenlib" ]; then
    source ../emsdk/emsdk_env.sh
    cd $BUILD_DIR
    emcmake cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF" \
        ${BOOST_PREFIX:+"-D Boost_ROOT=$BOOST_PREFIX"} \
        ../../..
    emmake make -j$(sysctl -n hw.logicalcpu)
    if [ -z $NO_INSTALL ]; then
        cd ../../..
        mkdir -p "src/npmpackage/build" || true
        cp "$BUILD_DIR/ocl.js" src/npmpackage/build/ || true
    fi
else
    echo "Usage: ./scripts/build-macos.sh lib build_type [clean]"
    echo "  lib: one of cxxlib, nodejslib, python3lib, emscriptenlib, all"
    echo "  build_type: one of debug, release"
    echo "  clean: optional, removes the build directory before building"
fi

