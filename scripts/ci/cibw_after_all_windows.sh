#!/bin/bash -xe

BOOST_ROOT_UNIX=$(cygpath -w "${BOOST_ROOT}")

rm "${BOOST_ROOT_UNIX}/boost_1_80_0.tar.gz"
tar -czf /d/a/opencamlib/opencamlib/boost.tar.gz "${BOOST_ROOT_UNIX}"
