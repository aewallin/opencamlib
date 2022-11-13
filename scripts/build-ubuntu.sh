#!/bin/bash -xe

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

if [ "$1" = "cxxlib" ]; then
    cd $BUILD_DIR
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_CXX_LIB="ON" \
        ../../..
    cmake --build . -j$(nproc)
elif [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
    cd ../..
    if [ "$2" = "release" ]; then
        ./src/nodejslib/node_modules/.bin/cmake-js build --out "${BUILD_DIR}" --CDBUILD_NODEJS_LIB="ON"
        cp -r $BUILD_DIR/Release/* src/npmpackage/build/Release || true
    else
        ./src/nodejslib/node_modules/.bin/cmake-js build --out "${BUILD_DIR}" --CDBUILD_NODEJS_LIB="ON" --debug
        cp -r $BUILD_DIR/Debug/* src/npmpackage/build/Debug || true
    fi
elif [ "$1" = "python3lib" ]; then
    cd $BUILD_DIR
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_PY_LIB="ON" \
        ../../..
    cmake --build . -j$(nproc)
elif [ "$1" = "emscriptenlib" ]; then
    source ../emsdk/emsdk_env.sh
    cd $BUILD_DIR
    emcmake cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF" \
        ../../..
    emmake make -j$(nproc)
else
    echo "Usage: ./scripts/build-macos.sh lib build_type [clean]"
    echo "  lib: one of cxxlib, nodejslib, python3lib, emscriptenlib"
    echo "  build_type: one of debug, release"
    echo "  clean: optional, removes the build directory before building"
fi

