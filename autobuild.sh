#!/bin/bash

set -e

if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build && cmake .. && make

cd ..

if [ ! -d /usr/include/mymuduo ]; then 
    mkdir /usr/include/mymuduo
fi

for header in `pwd`/src/include/*.h
do
    cp -f $header /usr/include/mymuduo
done

cp -f `pwd`/lib/libmymuduo.so /usr/lib

ldconfig