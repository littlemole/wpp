#!/bin/bash
set -e

cd /usr/local/src/
git clone https://github.com/littlemole/$1.git

repro=9801f3bee48f41630ef093e4a5368ad9bdb23c95
diy=e3f22f360ec048caed1d61bbfa8b8ed9d54836a0
cryptoneat=477d46a2d12de4698a7a1423d80ff8b49769d28c
patex=1903d80ed0c8f0fa567cd75bdbe15995513cb72c
metacpp=ef5bff82dfffe53bb6e2bcd0f3b0fc37251dab64

VERSION=$(eval echo -n \$$1)
#git checkout $VERSION

echo "WITH_TEST=$WITH_TEST"

/usr/local/bin/build.sh $1 $VERSION
