#!/bin/bash

cd ../..
rm -rf buildpythonlib || true
mkdir buildpythonlib
cd buildpythonlib
export CXX="/usr/local/opt/llvm/bin/clang++"
cmake ../src \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_CXX_LIB="OFF" \
  -DBUILD_PY_LIB="ON"
make -j4
sudo make install