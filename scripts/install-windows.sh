#!/bin/sh -xe

choco install wget --no-progress

wget -nv -O boost.zip https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.zip
7z x boost.zip -y -o/c/boost -r
cd /c/boost/boost_1_75_0
if [ "$1" = "python3lib" ]; then
    ./bootstrap.bat --with-python-version=3.9
elif [ "$1" = "python2lib" ]; then
    ./bootstrap.bat --with-python-version=2.7
else
    ./bootstrap.bat
fi
# if [ "$1" = "python2lib" ]; then
#     echo 'using python : 2.7 : C:\\Python27\\python : C:\\Python27\\include : C:\\Python27\\libs ;' >> project-config.jam
# elif [ "$1" = "python3lib" ]; then
#     echo 'using python : 3.9 : C:\\Python39\\python : C:\\Python39\\include : C:\\Python39\\libs ;' >> project-config.jam
# fi

./b2 address-model=64 threading=multi --with-python -j2 variant=$2 link=static --layout=system
ls /c/boost/boost_1_75_0/stage/lib

if [ "$1" = "emscriptenlib" ]; then
    cd ..
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
fi