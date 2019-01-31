#!/bin/bash

cd ../..
# rm -rf build

export CXX="/usr/local/opt/llvm/bin/clang++"

# cmake-js \
#     --out buildnodejsrelease \
#     compile \
#     --CDBUILD_NODEJS_LIB="ON" \
#     --CDBUILD_CXX_LIB="OFF"
# cp -r buildnodejsrelease/Release/* src/npmpackage/build/Release/ || true

cmake-js \
    --out buildnodejsdebug \
    compile \
    --debug \
    --CDBUILD_NODEJS_LIB="ON" \
    --CDBUILD_CXX_LIB="OFF"
cp -r buildnodejsdebug/Debug/* src/npmpackage/build/Debug/ || true