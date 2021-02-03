#!/bin/bash

build_type=$1
hypertrie_compiler=$2
compiler_version=$3

set -e
# change to parent dir of this script
cd "$(dirname "$0")/.."

command_layout="build.sh [build_type: {Debug, Release}] [hypertrie_compiler: {clang, gcc}] [compiler version: {10,11,..}] "

if [ "$1" == "-h" ] || [ "$1" = "--help" ] ; then
  echo "command:"
  echo "${command_layout}"
  exit 0
fi


if [ -z "${build_type}" ]; then
  build_type=Debug
fi
if [ "${build_type}" = "Debug" ] || [ "${build_type}" = "Release" ]; then
  echo "Do a ${build_type} build."
else
  echo "release type ${build_type} is not supported"
  echo "command:"
  echo "${command_layout}"
  exit 1
fi

if [ -z "${hypertrie_compiler}" ]; then
  hypertrie_compiler=clang
fi
if [ "${hypertrie_compiler}" = "clang" ] || [ "${hypertrie_compiler}" = "gcc" ]; then
  echo "Use ${hypertrie_compiler} as compiler."
else
  echo "compiler ${hypertrie_compiler} is not supported"
  echo "command:"
  echo "${command_layout}"
  exit 1
fi



if [ "${hypertrie_compiler}" = "clang" ]; then
  if [ -z "${compiler_version}" ]; then
    export CXX="clang++" CC="clang"
  else
    export CXX="clang++-${compiler_version}" CC="clang-${compiler_version}"
  fi
else
  if [ -z "${compiler_version}" ]; then
    export CXX="g++" CC="gcc"
  else
    export CXX="g++-${compiler_version}" CC="gcc-${compiler_version}"
  fi
fi

mkdir -p build
cd build
~/.local/bin/conan install .. --build=missing
cmake -Dhypertrie_BUILD_TESTS=ON -Dhypertrie_LIBTORCH_PATH=../libtorch -DCMAKE_BUILD_TYPE="${build_type}" ..
make
