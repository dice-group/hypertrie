# hypertrie - a flexible tensor sparse, low-dimensional tensor data structure

## build
### prerequisites

install conan, cmake and a C++17 compiler

add conan remotes
```shell script
conan remote add tsl https://api.bintray.com/conan/tessil/tsl && conan remote add public-conan https://api.bintray.com/conan/bincrafters/public-conan && conan remote add stiffstream https://api.bintray.com/conan/stiffstream/public
```

### build

```shell script
mkdir build
cd build
conan install .. --build=missing --settings compiler.libcxx="libstdc++11"
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j
```

# running tests with libtorch
Provide cmake with the path to your libtorch binaries:
```shell script
cmake -DCMAKE_PREFIX_PATH=*the path to libtorch* -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ..
make -j
```
