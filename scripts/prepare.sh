#!/bin/bash

set -e

~/.local/bin/conan user #conan setup
mkdir -p ~/.conan/profiles/
cp clang10_conan_profile ~/.conan/profiles/default
~/.local/bin/conan remote add -f tsl https://api.bintray.com/conan/tessil/tsl
~/.local/bin/conan remote add -f dice-group https://api.bintray.com/conan/dice-group/tentris
#conan remote add public-conan https://api.bintray.com/conan/bincrafters/public-conan
#conan remote add stiffstream https://api.bintray.com/conan/stiffstream/public
# install libtorch
if [ ! -d libtorch ]; then
    mkdir -p ~/.cache/libtorch/
    wget --continue --timestamping -O ~/.cache/libtorch/libtorch-cxx11-abi-shared-with-deps-1.4.0+cpu.zip https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.4.0%2Bcpu.zip
    unzip ~/.cache/libtorch/libtorch-cxx11-abi-shared-with-deps-1.4.0+cpu.zip
fi
