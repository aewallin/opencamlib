#!/bin/sh -xe

export HOMEBREW_NO_INSTALL_CLEANUP=1

if ! command -v brew &> /dev/null
then
    echo "brew could not be found in PATH, please install it, see: https://brew.sh"
    exit
fi

if [ "$1" = "python3lib" ]; then
    brew install libomp boost python3 boost-python3
else
    brew install libomp boost
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