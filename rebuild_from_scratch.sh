#!/usr/bin/env sh
trash -rf ./build
mkdir ./build
cd ./build
cmake ..
make -j8

