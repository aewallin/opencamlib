#!/bin/bash -xe

if [ "$1" = "all" ]; then
    ./scripts/build-windows.sh cxxlib debug $3
    ./scripts/build-windows.sh cxxlib release $3
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

if [ "$3" = "2019" ]; then
    GENERATOR="Visual Studio 16 2019"
else
    GENERATOR="Visual Studio 17 2022"
fi

if [ "$1" = "cxxlib" ]; then
    cd $BUILD_DIR
    cmake \
        -G "${GENERATOR}" \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_CXX_LIB="ON" \
        -D BOOST_ROOT="/c/boost/boost_1_80_0" \
        ../../..
    cmake --build . --config "${BUILD_TYPE}" -- -maxcpucount
elif [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
    cd ../..
    if [ "$2" = "release" ]; then
        ./src/nodejslib/node_modules/.bin/cmake-js \
            build \
            --out "${BUILD_DIR}" \
            --CD BUILD_NODEJS_LIB="ON" \
            --CD BOOST_ROOT="/c/boost/boost_1_80_0"
        cp -r $BUILD_DIR/Release/* src/npmpackage/build/Release || true
    else
        ./src/nodejslib/node_modules/.bin/cmake-js \
            build \
            --out "${BUILD_DIR}" \
            --CD BUILD_NODEJS_LIB="ON" \
            --CD BOOST_ROOT="/c/boost/boost_1_80_0" \
            --debug
        cp -r $BUILD_DIR/Debug/* src/npmpackage/build/Debug || true
    fi
elif [ "$1" = "python3lib" ]; then
    cd $BUILD_DIR
    cmake \
        -G "${GENERATOR}" \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_PY_LIB="ON" \
        -D BOOST_ROOT="/c/boost/boost_1_80_0" \
        ../../..
    cmake --build . --config "${BUILD_TYPE}" -- -maxcpucount
elif [ "$1" = "emscriptenlib" ]; then
    source ../emsdk/emsdk_env.sh
    cd $BUILD_DIR
    emcmake cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF" \
        -D BOOST_ROOT="/c/boost/boost_1_80_0" \
        ../../..
    emmake make
else
    echo "Usage: ./scripts/build-windows.sh lib build_type [clean]"
    echo "  lib: one of cxxlib, nodejslib, python3lib, emscriptenlib"
    echo "  build_type: one of debug, release"
    echo "  clean: optional, removes the build directory before building"
fi

