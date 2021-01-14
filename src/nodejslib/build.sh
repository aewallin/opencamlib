#!/bin/bash

cd ../..

mkdir -p src/npmpackage/build/{Debug,Release} || true

if [ "$1" = "release" ]; then
    ./node_modules/.bin/cmake-js build --out buildnodejsrelease --CDBUILD_NODEJS_LIB="ON"
    cp -r buildnodejsrelease/Release/* src/npmpackage/build/Release || true
else
    ./node_modules/.bin/cmake-js build --out buildnodejsdebug --CDBUILD_NODEJS_LIB="ON" --debug
    cp -r buildnodejsdebug/Debug/* src/npmpackage/build/Debug || true
fi
