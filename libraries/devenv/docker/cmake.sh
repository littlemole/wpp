#!/bin/bash
set -e

VERSION=3.18.4

apt remove --purge --auto-remove cmake

cd /usr/local/src/
wget "https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-$VERSION.tar.gz"

tar xzf "cmake-$VERSION.tar.gz"
cd "cmake-$VERSION"

./bootstrap
make -j$(nproc)
sudo make install

cmake --version
