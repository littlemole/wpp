#!/bin/bash
set -e

mkdir -p /usr/local/src
cd /usr/local/src/

git clone https://github.com/sheredom/utest.h.git utest

cd utest

mkdir -p /usr/local/include
cp utest.h /usr/local/include/



