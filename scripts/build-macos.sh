#!/bin/bash -xe

if [ "$1" = "all" ]; then
    ./scripts/build-macos.sh cxxlib debug $3
    ./scripts/build-macos.sh cxxlib release $3
    ./scripts/build-macos.sh python3lib debug $3
    ./scripts/build-macos.sh python3lib release $3
    ./scripts/build-macos.sh nodejslib debug $3
    ./scripts/build-macos.sh nodejslib release $3
    ./scripts/build-macos.sh emscriptenlib debug $3
    ./scripts/build-macos.sh emscriptenlib release $3
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

# export VERBOSE=1
if [ "$1" = "cxxlib" ]; then
    cd $BUILD_DIR
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_CXX_LIB="ON" \
        ../../..
    cmake --build . -j$(sysctl -n hw.logicalcpu)
elif [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
    cd ../..
    if [ "$2" = "release" ]; then
        ./src/nodejslib/node_modules/.bin/cmake-js build \
            --out "${BUILD_DIR}" \
            --parallel $(sysctl -n hw.logicalcpu) \
            --CD BUILD_NODEJS_LIB="ON"

        cp -r $BUILD_DIR/Release/* src/npmpackage/build/Release || true
    else
        ./src/nodejslib/node_modules/.bin/cmake-js build \
            --out "${BUILD_DIR}" \
            --parallel $(sysctl -n hw.logicalcpu) \
            --CD BUILD_NODEJS_LIB="ON" \
            --debug

        cp -r $BUILD_DIR/Debug/* src/npmpackage/build/Debug || true
    fi
elif [ "$1" = "python3lib" ]; then
    cd $BUILD_DIR
    cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D CMAKE_FIND_USE_CMAKE_PATH="OFF" \
        -D Python_FIND_STRATEGY="LOCATION" \
        -D Python3_ROOT_DIR="$(brew --prefix python3)" \
        -D BUILD_PY_LIB="ON" \
        ../../..
    cmake --build . -j$(sysctl -n hw.logicalcpu)
    cmake --install .
elif [ "$1" = "emscriptenlib" ]; then
    source ../emsdk/emsdk_env.sh
    cd $BUILD_DIR
    emcmake cmake \
        -D CMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF" \
        ../../..
    emmake make -j$(sysctl -n hw.logicalcpu)
    cd ../../..
    cp -r "$BUILD_DIR/src/opencamlib.js" "src/npmpackage/build/$BUILD_TYPE" || true
else
    echo "Usage: ./scripts/build-macos.sh lib build_type [clean]"
    echo "  lib: one of cxxlib, nodejslib, python3lib, emscriptenlib"
    echo "  build_type: one of debug, release"
    echo "  clean: optional, removes the build directory before building"
fi

