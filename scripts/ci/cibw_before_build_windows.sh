#!/bin/bash -xe

BOOST_ROOT_UNIX=$(cygpath -w "${BOOST_ROOT}")
BOOST_ROOT_DIR=$(dirname "${BOOST_ROOT_UNIX}")

if [ -f "${BOOST_ROOT_UNIX}/boost_1_80_0.tar.gz" ]; then
  cd "${BOOST_ROOT_UNIX}/boost_1_80_0"
  ./bootstrap.bat
  PYTHON_VERSION=$(python -c 'import sys; print(str(sys.version_info.major) + "." + str(sys.version_info.minor))')
  PYTHON_H_LOCATION=$(python -c 'import sysconfig; print(sysconfig.get_config_h_filename())')
  PYTHON_INCLUDE_DIR=$(dirname "${PYTHON_H_LOCATION}")
  PYTHON_INCLUDE_DIR_FIXED=$(cygpath -w "${PYTHON_INCLUDE_DIR}" | sed 's/\\/\\\\/g')
  echo "using python : ${PYTHON_VERSION} : : ${PYTHON_INCLUDE_DIR_FIXED} ;" > ./user-config.jam
  cat ./user-config.jam
  ./b2 \
    -a \
    threading=multi \
    -j2 \
    variant="release" \
    link=static \
    ${BOOST_ADDRESS_MODEL:+"address-model=${BOOST_ADDRESS_MODEL}"} \
    ${BOOST_ARCHITECTURE:+"architecture=${BOOST_ARCHITECTURE}"} \
    --layout=system \
    --with-python \
    --user-config=./user-config.jam \
    stage
fi
