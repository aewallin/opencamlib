#!/bin/bash -xe

armloc=$(brew fetch --bottle-tag=arm64_big_sur libomp | grep -i downloaded | grep tar.gz | cut -f2 -d ":" | xargs echo)
x64loc=$(brew fetch --bottle-tag=big_sur libomp | grep -i downloaded | grep tar.gz | cut -f2 -d ":" | xargs echo)
cp $armloc /tmp/libomp-arm64.tar.gz
mkdir /tmp/libomp-arm64 || true
tar -xzvf /tmp/libomp-arm64.tar.gz -C /tmp/libomp-arm64
LIBOMP_PREFIX=$(find /tmp/libomp-arm64/libomp -depth 1 | head -1)
mv "${LIBOMP_PREFIX}" "/tmp/libomp-arm64/libomp/fixed"

cp $x64loc /tmp/libomp-x86_64.tar.gz
mkdir /tmp/libomp-x86_64 || true
tar -xzvf /tmp/libomp-x86_64.tar.gz -C /tmp/libomp-x86_64
LIBOMP_PREFIX=$(find /tmp/libomp-x86_64/libomp -depth 1 | head -1)
mv "${LIBOMP_PREFIX}" "/tmp/libomp-x86_64/libomp/fixed"

BOOST_ROOT_DIR=$(dirname "${BOOST_ROOT}")

if [ -f boost.tar.gz ]; then
  echo "Found boost.tar.gz, assuming it is a valid cache, using it..."
  tar -C "${BOOST_ROOT_DIR}/" -xzf boost.tar.gz
else
  if ! command -v wget &> /dev/null; then
    brew install wget
  fi
  cd /tmp
  mkdir boost
  cd boost
  wget -q --no-check-certificate 'https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz' 
  tar zxf boost_1_80_0.tar.gz
fi