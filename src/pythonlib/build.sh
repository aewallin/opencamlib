#!/bin/bash

cd ../..

# rm -rf buildpythonlib || true
mkdir buildpythonlib || true
cd buildpythonlib

cmake ../src \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_PY_LIB="ON"

make -j4

sudo make install