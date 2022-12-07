#!/bin/bash -xe

BOOST_ROOT_UNIX=$(cygpath -w "${BOOST_ROOT}")
BOOST_ROOT_DIR=$(dirname "${BOOST_ROOT_UNIX}")

if [ -f boost.tar.gz ]; then
  echo "Found boost.tar.gz, assuming it is a valid cache, using it..."
  tar -C "$BOOST_ROOT_DIR/" -xzf boost.tar.gz
else
  if ! command -v wget &> /dev/null; then
    choco install wget --no-progress
  fi
  cd "${BOOST_ROOT_DIR}"
  mkdir boost
  cd boost
  wget -q --no-check-certificate 'https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz' 
  tar zxf boost_1_80_0.tar.gz
fi
