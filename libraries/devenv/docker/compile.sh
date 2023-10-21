#!/bin/bash
set -e

echo "**********************************"
echo "compiling $1 for $BACKEND with" 
echo "$CXX using $BUILDCHAIN"
echo "**********************************"

function cmake_build {

    MODE="$1"
    
    mkdir -p $MODE
    cd $MODE

    if [ "$BACKEND" == "libevent" ]
    then
        cmake .. -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_BUILD_TYPE=$MODE -DWITH_LIBEVENT=On
    else
        cmake .. -DCMAKE_CXX_COMPILER=$CXX -DWITH_LIBEVENT=Off -DCMAKE_BUILD_TYPE=$MODE
    fi
     
    make
    
    if [ "$WITH_TEST" == "Off" ]
    then
    	echo "skipping tests for $1 ..."
    else
        ctest
    fi
    
    cd ..
}


cd /usr/local/src/$1

cmake_build "Debug"
cmake_build "Release"


