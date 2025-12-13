#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -eq 0 ]; then
  set -- demo/main.ys
fi

cmake -S . -B build
cmake --build build
./build/yuki2d "$@"
