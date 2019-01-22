#!/bin/sh

rm -rf ../../buildemscripten || true
mkdir ../../buildemscripten
cd ../../buildemscripten
emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CPP_LIB="OFF" \
    -DBUILD_EMSCRIPTEN_LIB="ON" \
    -DUSE_OPENMP="OFF" \
    -DBoost_INCLUDE_DIR="/usr/local/Cellar/boost/1.68.0/include"
emmake make -j4
cp src/opencamlib.* ../src/emscriptenlib
cd ../src/emscriptenlib
./node_modules/.bin/browserify test.src.js > test.js
./node_modules/.bin/browserify index.js > pkg/index.js
