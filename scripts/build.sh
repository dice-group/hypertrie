#!/bin/bash

build_type=$1
hypertrie_compiler=$2
compiler_version=$3

set -e
# change to parent dir of this script
cd "$(dirname "$0")/.."
source scripts/internal/parse_commandline_args.sh

parse_commandline_args "${build_type}" "${hypertrie_compiler}" "${compiler_version}"

mkdir -p build
cd build
~/.local/bin/conan install .. --build=missing
cmake -Dhypertrie_BUILD_TESTS=ON -Dhypertrie_LIBTORCH_PATH=../libtorch -DCMAKE_BUILD_TYPE="${build_type}" ..
make
