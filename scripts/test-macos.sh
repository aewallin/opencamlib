#!/bin/bash -xe

if [ "$1" = "cxxlib" ]; then
  cd examples/cpp/test
  mkdir build || true
  cd build
  cmake ..
  cmake --build .
  ./test_example
fi

if [ "$1" = "nodejslib" ]; then
  cd src/npmpackage
  npm install
  npm run build-node
  cd ../../examples/nodejs
  npm link ../../src/npmpackage
  if [ "$2" = "debug" ]; then
    export DEBUG=1
  fi
  node test.js
fi

if [ "$1" = "emscriptenlib" ]; then
  cd src/npmpackage
  npm install
  npm run build-emscripten
  cd ../../examples/emscripten
  npm link ../../src/npmpackage
  npm start &
  SERVER_PID=$!
  sleep 3
  node test.js
  kill ${SERVER_PID}
fi

if [ "$1" = "python3lib" ]; then
  cd examples/python
  ${OCL_PYTHON_EXECUTABLE:-python3} -m sysconfig
  ${OCL_PYTHON_EXECUTABLE:-python3} test.py
fi
