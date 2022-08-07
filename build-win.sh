#!/bin/bash

DIR=$(dirname $0)
if [ "$DIR" = "" ]; then
    DIR="."
fi

if ! [ -d $DIR/build/ ]; then
    rm -rf $DIR/build/
fi

mkdir $DIR/build
cd $DIR/build
mingw64-cmake ..
mingw64-make install -j$(nproc)
