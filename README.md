TODO: update readme

# hypertrie

A flexible data structure for low-rank, sparse tensors supporting slices by any dimension and einstein summation (
einsum).

For details on the data structure refer to https://tentris.dice-research.org/

## build

### prerequisites

install conan, cmake and a C++20 compiler.

and create a conan profile

 ```shell script
conan profile new --detect default
conan profile update settings.compiler.libcxx=libstdc++11 default
 ```

You'll need some packages from DICE group's conan artifactory. Add it with:

```shell script
conan remote add dice-group https://conan.dice-research.org/artifactory/api/conan/tentris
```

### build

```shell script
mkdir build
cd build
conan install .. --build=missing
cmake ..
```

# running tests

To enable test, set `DBUILD_TESTING` in cmake:

```shell script
cmake -DBUILD_TESTING=ON ..
make -j tests
tests/tests
```

Some tests are using [pytorch](https://github.com/pytorch/pytorch) which is not provided with the code.
Those tests are disabled by default.
To enable them, provide the path to the pytorch library via cmake variable `hypertrie_LIBTORCH_PATH`.
Prebuild binaries may be downloaded via https://pytorch.org/get-started/locally/ (works at least with
Stable|Linux|LibTorch|C++|None).

```shell script
cmake -DDBUILD_TESTING=ON -DLIBTORCH_PATH=/path/to/libtorch ..
```
