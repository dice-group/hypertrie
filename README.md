# hypertrie
A flexible data structure for low-rank, sparse tensors supporting slices by any dimension and einstein summation (einsum). 

For details on the data structure refer to https://tentris.dice-research.org/


## build
### prerequisites

install conan, cmake and a C++20 compiler. The steps below are tested for gcc 10, clang 10 and clang 11.

Add tsl remote to conan 
```shell script
conan remote add tsl https://api.bintray.com/conan/tessil/tsl
```
and create a conan profile
 ```shell script
conan profile new --detect default
conan profile update settings.compiler.libcxx=libstdc++11 default
 ```
We recommend to use this profile https://github.com/dice-group/hypertrie/blob/development/clang10_conan_profile with clang-10 or to adapt it to your C++20 compiler.

### build

```shell script
mkdir build
cd build
conan install .. --build=missing
cmake ..
```

# running tests
To enable test, set `hypertrie_BUILD_TESTS` in cmake:
```shell script
cmake -Dhypertrie_BUILD_TESTS=ON ..
make -j tests
tests/tests
```
Some tests are using [pytorch](https://github.com/pytorch/pytorch) which is not provided with the code.
Those tests are disabled by default. 
To enable them, provide the path to the pytorch library via cmake variable `hypertrie_LIBTORCH_PATH`.
Prebuild binaries may be download via https://pytorch.org/get-started/locally/ (works at least with Stable|Linux|LibTorch|C++|None).
```shell script
cmake -Dhypertrie_BUILD_TESTS=ON -Dhypertrie_LIBTORCH_PATH=/path/to/libtorch ..
```
