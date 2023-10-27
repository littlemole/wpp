#!/bin/bash
set -e


cd /usr/src/googletest/googletest


if [ "$CXX" == "g++" ]
then 
	FLAGS=""
else
	FLAGS="-stdlib=libc++"
fi

cmake -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_CXX_FLAGS=$FLAGS –DCMAKE_CXX_STANDARD=20  . 
make
  
cp /usr/src/googletest/googletest/lib/libgtest.a /usr/lib/x86_64-linux-gnu/libgtest.a && 
cp /usr/src/googletest/googletest/lib/libgtest_main.a /usr/lib/x86_64-linux-gnu/libgtest_main.a

