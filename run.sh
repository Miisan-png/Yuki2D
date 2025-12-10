#!/usr/bin/env bash
mkdir -p build
cd build
cmake ..
cmake --build .
./yuki2d ../scripts/example_main.ys
