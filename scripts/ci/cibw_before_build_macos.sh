#!/bin/bash -xe

BOOST_ROOT_DIR=$(dirname "${BOOST_ROOT}")

if [ -f "${BOOST_ROOT}/boost_1_80_0.tar.gz" ]; then
  cd "${BOOST_ROOT}/boost_1_80_0"
  ./bootstrap.sh
  PYTHON_VERSION=$(python -c 'import sys; print(str(sys.version_info.major) + "." + str(sys.version_info.minor))')
  PYTHON_BIN=$(which python)
  PYTHON_INCLUDE_DIR=$(python -c "from sysconfig import get_path; print(get_path('include'))")
  echo "using python : ${PYTHON_VERSION} : ${PYTHON_BIN} : ${PYTHON_INCLUDE_DIR} ;" >> ./user-config.jam
  cat ./user-config.jam
  ./b2 \
    -a \
    threading=multi \
    -j$(sysctl -n hw.logicalcpu) \
    variant="release" \
    link=static \
    address-model=64 \
    ${BOOST_ADDRESS_MODEL:+"address-model=${BOOST_ADDRESS_MODEL}"} \
    ${BOOST_ARCHITECTURE:+"architecture=${BOOST_ARCHITECTURE}"} \
    --layout=system \
    --with-python \
    --user-config=./user-config.jam \
    cxxflags='-fPIC' \
    stage
fi