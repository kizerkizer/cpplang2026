#!/usr/bin/env bash
set -e

if [ "$1" == "clean" ]; then
  rm -rf out
fi

cmake -S . -B out -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build out
