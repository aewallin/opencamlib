#!/bin/bash -xe

export PATH="$PATH:/c/Program Files/nodejs:/c/Python27/python:/c/Python39/python"

if [ "$1" = "all" ]; then
    ./scripts/build-windows.sh cxxlib debug $3
    ./scripts/build-windows.sh cxxlib release $3
    ./scripts/build-windows.sh python2lib debug $3
    ./scripts/build-windows.sh python2lib release $3
    ./scripts/build-windows.sh python3lib debug $3
    ./scripts/build-windows.sh python3lib release $3
    ./scripts/build-windows.sh nodejslib debug $3
    ./scripts/build-windows.sh nodejslib release $3
    ./scripts/build-windows.sh emscriptenlib debug $3
    ./scripts/build-windows.sh emscriptenlib release $3
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

mkdir -p $BUILD_DIR || true

if [ "$3" = "2016" ]; then
    GENERATOR="Visual Studio 15 2017"
else
    GENERATOR="Visual Studio 16 2019"
fi

if [ "$1" = "cxxlib" ]; then
    cd $BUILD_DIR
    cmake \
        -G "${GENERATOR}" \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_CXX_LIB="ON" \
        -D BOOST_ROOT="/c/boost/boost_1_75_0" \
        ../../..
    cmake --build .  -- -maxcpucount
elif [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
    cd ../..
    if [ "$2" = "release" ]; then
        ./src/nodejslib/node_modules/.bin/cmake-js \
            build \
            --out "${BUILD_DIR}" \
            --CD BUILD_NODEJS_LIB="ON" \
            --CD BOOST_ROOT="/c/boost/boost_1_75_0"
        # cp -r $BUILD_DIR/Release/* src/npmpackage/build/Release || true
    else
        ./src/nodejslib/node_modules/.bin/cmake-js \
            build \
            --out "${BUILD_DIR}" \
            --CD BUILD_NODEJS_LIB="ON" \
            --CD BOOST_ROOT="/c/boost/boost_1_75_0" \
            --debug
        # cp -r $BUILD_DIR/Debug/* src/npmpackage/build/Debug || true
    fi
elif [ "$1" = "python2lib" ]; then
    cd $BUILD_DIR
    cmake \
        -G "${GENERATOR}" \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_PY_LIB="ON" \
        -D USE_PY_3="OFF" \
        -D BOOST_ROOT="/c/boost/boost_1_75_0" \
        ../../..
    cmake --build .  -- -maxcpucount
elif [ "$1" = "python3lib" ]; then
    cd $BUILD_DIR
    cmake \
        -G "${GENERATOR}" \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_PY_LIB="ON" \
        -D BOOST_ROOT="/c/boost/boost_1_75_0" \
        ../../..
    cmake --build .  -- -maxcpucount
elif [ "$1" = "emscriptenlib" ]; then
    source ../emsdk/emsdk_env.sh
    cd $BUILD_DIR
    # By default, find_package(Boost) breaks when using emscripten because it doesn't respect the paths / hints
    # We use CMAKE_FIND_ROOT_PATH="/" to work around this issue
    # See https://github.com/emscripten-core/emscripten/issues/10078
    emcmake cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D CMAKE_FIND_ROOT_PATH="/" \
        -D BOOST_ROOT="/c/boost/boost_1_75_0" \
        -D USE_OPENMP="OFF" \
        ../../..
    emmake make
else
    echo "Usage: ./scripts/build-windows.sh lib build_type [clean]"
    echo "  lib: one of cxxlib, nodejslib, python2lib, python3lib, emscriptenlib"
    echo "  build_type: one of debug, release"
    echo "  clean: optional, removes the build directory before building"
fi

