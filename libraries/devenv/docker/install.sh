#!/bin/bash
set -e

cd /usr/local/src/
git clone https://github.com/littlemole/$1.git

repro=4ac2dd63df4c29909b3250c583c975364271ea89
diy=4a47114d21e891e7af2052eb9b1da333cdddb50a
cryptoneat=70e0e2f133d51dc9c0bb040e3ada8cf9be8fa573
patex=de54c3b1d051627a6254bcf96a00b68118ab75d8
metacpp=7aa997e726ba3f374dc9f02ec3f6e0f5906e0111

VERSION=$(eval echo -n \$$1)
#git checkout $VERSION

echo "WITH_TEST=$WITH_TEST"

/usr/local/bin/build.sh $1 $VERSION
