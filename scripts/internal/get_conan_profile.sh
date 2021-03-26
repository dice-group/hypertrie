#!/bin/bash

function get_conan_profile() {
  # uses those variables:
  # - build_type
  # - hypertrie_compiler
  # - compiler_version
  # shellcheck disable=SC2034
  conan_profile="${build_type}_${hypertrie_compiler}-${compiler_version}_hypertrie_profile"
}
