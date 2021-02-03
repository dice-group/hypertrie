#!/bin/bash

build_type=$1
hypertrie_compiler=$2
compiler_version=$3

set -e
# change to parent dir of this script
cd "$(dirname "$0")/.."
source scripts/internal/parse_commandline_args.sh
source scripts/internal/get_conan_profile.sh

parse_commandline_args
get_conan_profile

mkdir -p build
cd build
~/.local/bin/conan install .. --build=missing --profile ${conan_profile}
cmake -Dhypertrie_BUILD_TESTS=ON -Dhypertrie_LIBTORCH_PATH=../libtorch -DCMAKE_BUILD_TYPE="${build_type}" ..
make
