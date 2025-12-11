#!/usr/bin/env bash
mkdir -p build
cd build
cmake ..
cmake --build .
./yuki2d ../demo/main.ys
