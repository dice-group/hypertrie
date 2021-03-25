#!/bin/bash

set -e
# change to parent dir of this script
cd "$(dirname "$0")/.."

echo "Executing tests (log redirected to file due to excessive logging)"
./build/tests/tests_einsum >tests.log

echo "Compressing test logs"
xz tests.log
