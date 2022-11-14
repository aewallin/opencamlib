#!/bin/sh -xe

sudo apt-get update

if [ "$1" = "python3lib" ]; then
    sudo apt-get install -y git cmake build-essential doxygen texlive-latex-base python3 python3-dev libboost-dev libboost-python-dev
else
    sudo apt-get install -y git cmake build-essential doxygen texlive-latex-base libboost-dev
fi

if [ "$1" = "nodejslib" ]; then
    cd src/nodejslib
    npm install
fi

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
fi
