#!/bin/bash

build_type=$1
hypertrie_compiler=$2
compiler_version=$3

set -e
# change to parent dir of this script
cd "$(dirname "$0")/.."
source scripts/internal/parse_commandline_args.sh
source scripts/internal/find_conan.sh

parse_commandline_args "${build_type}" "${hypertrie_compiler}" "${compiler_version}"
find_conan

conan_profile="${build_type}_${hypertrie_compiler}-${compiler_version}_hypertrie_profile"
if ! conan profile show "${conan_profile}"; then
  conan user
  conan profile new --detect "${conan_profile}"
  conan profile update settings.compiler.libcxx=libstdc++11 "${conan_profile}"
  conan profile update env.CXXFLAGS="${CXXFLAGS}" "${conan_profile}"
  conan profile update env.CMAKE_EXE_LINKER_FLAGS="${CMAKE_EXE_LINKER_FLAGS}" "${conan_profile}"
  conan profile update env.CXX="${CXX}" "${conan_profile}"
  conan profile update env.CC="${CC}" "${conan_profile}"
  conan profile update options.boost:extra_b2_flags="cxxflags=\\\"${CXXFLAGS}\\\"" "${conan_profile}"
fi

conan remote add -f tsl https://api.bintray.com/conan/tessil/tsl
conan remote add -f dice-group https://api.bintray.com/conan/dice-group/tentris

# install libtorch
if [ ! -d libtorch ]; then
  mkdir -p ~/.cache/libtorch/
  wget --continue --timestamping -O ~/.cache/libtorch/libtorch-cxx11-abi-shared-with-deps-1.4.0+cpu.zip https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.4.0%2Bcpu.zip
  unzip ~/.cache/libtorch/libtorch-cxx11-abi-shared-with-deps-1.4.0+cpu.zip
fi
