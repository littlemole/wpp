#!/bin/bash
set -e

VERSION=$2


echo "**********************************"
echo "building $1 for $BACKEND with" 
echo "$CXX using $BUILDCHAIN"
echo "**********************************"

function cmake_build {

    MODE="$1"
    
    mkdir -p $MODE
    cd $MODE


    if [ "$BACKEND" == ""  ]
    then
        cmake .. -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_BUILD_TYPE=$MODE -DWITH_TEST=$WITH_TEST     
    elif [ "$BACKEND" == "libevent" ]
    then
        cmake .. -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_BUILD_TYPE=$MODE -DWITH_LIBEVENT=On -DWITH_TEST=$WITH_TEST
    else
        cmake .. -DCMAKE_CXX_COMPILER=$CXX -DWITH_LIBEVENT=Off -DCMAKE_BUILD_TYPE=$MODE -DWITH_TEST=$WITH_TEST
    fi
     
    make
    
    if [ "$WITH_TEST" == "Off" ]
    then
    	echo "skipping tests for $1 ..."
    else
        ctest
    fi
    
    make install  
    cd ..
}


cd /usr/local/src/$1

if [ "$VERSION" != "" ]
then
    git checkout $VERSION
fi


cmake_build "Debug"
cmake_build "Release"


