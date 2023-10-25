#!/bin/bash
set -e

mkdir -p /usr/src
cd /usr/src/

git clone https://github.com/sheredom/utest.h.git utest

cd utest

mkdir -p /usr/local/include
cp utest.h /usr/local/include/



