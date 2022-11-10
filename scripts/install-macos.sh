#!/bin/sh -xe

export HOMEBREW_NO_INSTALL_CLEANUP=1

if [ "$1" = "python2lib" ]; then
    brew install llvm libomp boost boost-python python@2
elif [ "$1" = "python3lib" ]; then
    brew install llvm libomp boost boost-python3
else
    brew install llvm libomp boost
fi

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
fi