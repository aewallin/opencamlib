#!/bin/bash

wget -nv -O boost_1_80_0.tar.gz https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
tar -zxf boost_1_80_0.tar.gz

cd src/nodejslib
npm install
cd ../..

# somehow git is not working in the docker container, or perhaps hidden files are not mounted.
git describe --tags > src/git-tag.txt

docker run --user root -w /work -v "$(pwd):/work:rw" --rm "${DOCKER_IMAGE}" ./scripts/build-linux.sh \
  nodejslib release \
  --use-openmp \
  --boost-prefix /work \
  --install-prefix /work/dist \
  --architecture arm64 \
  --no-install && \
  mkdir dist || true && \
  cp build/nodejslib/release/ocl.node dist/