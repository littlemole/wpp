#!/bin/bash
set -e

if [ "$BUILDCHAIN" == "make" ]
then

cd /usr/src/

git clone https://github.com/open-source-parsers/jsoncpp.git

cd jsoncpp

if [ "$CXX" == "g++" ]
then 
	FLAGS=""
else
	FLAGS="-stdlib=libc++"
fi

mkdir -p build
cd build
cmake -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_CXX_FLAGS=$FLAGS  –DCMAKE_CXX_STANDARD=20 -DBUILD_STATIC_LIBS=On -DBUILD_SHARED_LIBS=Off -DJSONCPP_WITH_WARNING_AS_ERROR=Off -DJSONCPP_WITH_TESTS=Off ..
make
make install
#rm /usr/local/lib/libjsoncpp.so*
  
fi

