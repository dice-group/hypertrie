#!/bin/bash

set -e

#if [ "$CXX" = "g++" ]; then export CXX="clang++-10" CC="clang-10"; fi

export CXX="clang++-10" CC="gcc-10"

mkdir -p build
cd build
~/.local/bin/conan install .. --build=missing
#cmake -Dhypertrie_BUILD_TESTS=ON -Dhypertrie_LIBTORCH_PATH=../libtorch -DCMAKE_BUILD_TYPE=Debug ..
cmake -Dhypertrie_BUILD_TESTS=ON -Dhypertrie_LIBTORCH_PATH=../libtorch -DCMAKE_BUILD_TYPE=Release ..
make

