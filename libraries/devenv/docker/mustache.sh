#!/bin/bash
set -e

mkdir -p /usr/local/src
cd /usr/local/src/

git clone https://github.com/kainjow/Mustache.git

cd Mustache

cp mustache.hpp /usr/local/include/


