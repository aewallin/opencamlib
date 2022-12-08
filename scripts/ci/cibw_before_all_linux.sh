#!/bin/bash

# bump python version to current date
sed -i.bak "s/^version = .*/version = \"$(date '+%Y.%-m.%-d')\"/g" pyproject.toml && rm pyproject.toml.bak

PROJECT_DIR=$(pwd)

if [ -f boost.tar.gz ]; then
  echo "Found boost.tar.gz, assuming it is a valid cache, using it..."
  tar -C / -xzf boost.tar.gz
else
  yum install -y wget
  cd /
  mkdir boost
  cd boost
  wget -q --no-check-certificate 'https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz'
  tar zxf boost_1_80_0.tar.gz
  cd boost_1_80_0/libs/python
  git apply "${PROJECT_DIR}/.github/patches/boost-python-3.11.patch"
  cd ../..
  ./bootstrap.sh
  echo "using python : 3.7 ;" >> ./user-config.jam
  echo "using python : 3.8 ;" >> ./user-config.jam
  echo "using python : 3.9 ;" >> ./user-config.jam
  echo "using python : 3.10 ;" >> ./user-config.jam
  echo "using python : 3.11 ;" >> ./user-config.jam
  cat ./user-config.jam
  ./b2 \
    -a \
    threading=multi \
    -j$(nproc) \
    variant="release" \
    link=static \
    address-model=64 \
    ${BOOST_ADDRESS_MODEL:+"address-model=${BOOST_ADDRESS_MODEL}"} \
    ${BOOST_ARCHITECTURE:+"architecture=${BOOST_ARCHITECTURE}"} \
    --layout=system \
    --with-python \
    python="3.7,3.8,3.9,3.10,3.11" \
    --user-config=./user-config.jam \
    cxxflags='-fPIC' \
    stage
  rm /boost/boost_1_80_0.tar.gz
  tar -czf /host/home/runner/work/opencamlib/opencamlib/boost.tar.gz /boost
fi