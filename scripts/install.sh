#!/bin/bash

set -e

sudo apt update
sudo apt install -y g++-10 clang-10 make cmake python3-pip python3-setuptools python3-wheel

pip3 install --user conan
