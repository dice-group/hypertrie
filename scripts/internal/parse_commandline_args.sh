#!/bin/bash

function parse_commandline_args() {
  # uses those variables:
  # - build_type
  # - hypertrie_compiler
  # - compiler_version
  command_layout="$(basename "$0") [build_type: {Debug, Release}] [hypertrie_compiler: {clang, gcc}] [compiler version: {10,11,..}] "

  if [ "${build_type}" == "-h" ] || [ "${build_type}" = "--help" ]; then
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
      compiler_version=10
    fi
    export CXX="clang++-${compiler_version}" CC="clang-${compiler_version}"
  else
    if [ -z "${compiler_version}" ]; then
      compiler_version=10
    fi
    export CXX="g++-${compiler_version}" CC="gcc-${compiler_version}"
  fi

}
