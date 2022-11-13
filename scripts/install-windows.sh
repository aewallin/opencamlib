#!/bin/sh -xe

choco install wget --no-progress

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
fi

wget -nv -O boost.zip https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.7z
7z x boost.zip -y -o/c/boost -r > nul
cd /c/boost/boost_1_80_0

if [ "$1" = "python3lib" ]; then
    ./bootstrap.bat
    if [ -n "${PYTHON_EXECUTABLE}" ]; then
        PYTHON_VERSION=`${PYTHON_EXECUTABLE} -c 'import sys; version=sys.version_info[:3]; print("{0}.{1}".format(*version))'`
        echo "using python : ${PYTHON_VERSION} : ${PYTHON_EXECUTABLE} ;" > ./project-config.jam
    fi
else
    ./bootstrap.bat
fi

if [ "$1" = "python3lib" ]; then
    ./b2 address-model=64 threading=multi --with-python -j2 variant=$2 link=static --layout=system install
else
    ./b2 address-model=64 threading=multi -j2 variant=$2 link=static --layout=system install
fi
