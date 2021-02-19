#!/bin/bash

build_type=$1
hypertrie_compiler=$2
compiler_version=$3

set -e
# change to parent dir of this script
cd "$(dirname "$0")/.."
source scripts/internal/parse_commandline_args.sh
source scripts/internal/find_conan.sh
source scripts/internal/get_conan_profile.sh

parse_commandline_args "${build_type}" "${hypertrie_compiler}" "${compiler_version}"

if ! type "${CC}"; then
  echo "C compiler ${CC} does not exist."
fi

if ! type "${CXX}"; then
  echo "C++ compiler ${CXX} does not exist."
fi

find_conan

get_conan_profile

conan user
if ! conan profile show "${conan_profile}"; then
  conan profile new --detect "${conan_profile}"
  conan profile update settings.compiler.libcxx=libstdc++11 "${conan_profile}"
  conan profile update env.CXXFLAGS="${CXXFLAGS}" "${conan_profile}"
  conan profile update env.CMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS}" "${conan_profile}"
  conan profile update env.CXX="${CXX}" "${conan_profile}"
  conan profile update env.CC="${CC}" "${conan_profile}"
fi

conan remote add -f tsl https://api.bintray.com/conan/tessil/tsl
conan remote add -f dice-group https://api.bintray.com/conan/dice-group/tentris

# install libtorch
if [ ! -d libtorch ]; then
  mkdir -p ~/.cache/libtorch/
  wget --continue --timestamping -O ~/.cache/libtorch/libtorch-cxx11-abi-shared-with-deps-1.4.0+cpu.zip https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.4.0%2Bcpu.zip
  unzip ~/.cache/libtorch/libtorch-cxx11-abi-shared-with-deps-1.4.0+cpu.zip
fi
