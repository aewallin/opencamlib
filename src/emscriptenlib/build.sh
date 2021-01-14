# #!/bin/bash

cd ../..

source ../emsdk/emsdk_env.sh

# rm -rf buildemscriptenlib || true
mkdir buildemscriptenlib || true
cd buildemscriptenlib

if [ "$1" == "release" ]; then
    emcmake cmake ../src \
        -D CMAKE_BUILD_TYPE="Release" \
        -D BUILD_CXX_LIB="OFF" \
        -D BUILD_PY_LIB="OFF" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF"
else
    emcmake cmake ../src \
        -D CMAKE_BUILD_TYPE="Debug" \
        -D BUILD_CXX_LIB="OFF" \
        -D BUILD_PY_LIB="OFF" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF"
fi

emmake make -j4

cp opencamlib.* ../src/npmpackage/