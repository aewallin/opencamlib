#!/bin/sh -xe

sudo apt-get update

sudo apt-get install -y python3 python3-dev libboost-dev libboost-python-dev

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
fi

if [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
    cd ../..
fi
