#!/bin/bash

set -e

echo "Executing tests (log redirected to file due to excessive logging)"
./build/bin/tests_einsum > tests.log

echo "Compressing test logs"
xz tests.log
