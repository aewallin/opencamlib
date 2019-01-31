#!/bin/bash

source ~/Projects/emsdk/emsdk_env.sh
rm -rf ../../buildemscripten || true
mkdir ../../buildemscripten
cd ../../buildemscripten
emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CXX_LIB="OFF" \
    -DBUILD_EMSCRIPTEN_LIB="ON" \
    -DUSE_OPENMP="OFF" \
    -DBoost_INCLUDE_DIR="/usr/local/Cellar/boost/1.68.0/include"
emmake make -j4
cp src/opencamlib.* ../src/npmpackage/