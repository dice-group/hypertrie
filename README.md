# Hypertrie

## Where is it used?

- the backend of the [commercial](https://github.com/tentris/tentris)
  and [research-prototype](https://github.com/dice-group/tentris-research-project) Tentris triple stores/RDF graph
  databases
- a sparse tensor representation that supports slicing by any dimension and einstein summation
- a monolithic index that supports worst-case optimal joins (WCOJ) by providing all collation orders in a single
  redundancy eliminating datastructure.

## What is it?

Technically, a hypertrie stores $d$-tuples where $d$ is also called dimension (tensor) or depth (trie, index).

It allows incremental slicing (tensor) by any dimension or select and project by any predicate (relational algebra).
These properties are important to support worst-case optimal joins (WCOJ) efficiently.

## Asymptotic Guarantees

A depth $d$ the hypertrie encoding a set of $z$ tuples requires at most $\mathcal O (z\cdot 2^{d-1}\cdot d)$ space. The
runtime complexity of applying (inserting or deleting) a changeset set $\Delta$ of $d$-tuples to a depth $d$ hypertrie
is bound by the space complexity $\mathcal O (|\Delta| \cdot 2^{d-1}\cdot d)$ of a surrogate hypertrie that encodes the
change set changeset $\Delta$.

# build

This is a template library. So there is nothing to build beyond tests.

## prerequisites

Software was tested on Ubuntu-22.04 with gcc-13 and clang-17, both using libstdc++-13 as C++ STL.

Install with:

```shell
# gcc-13
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
          
# clang-17
source /etc/os-release
echo "deb http://apt.llvm.org/${UBUNTU_CODENAME}/ llvm-toolchain-${UBUNTU_CODENAME}-17 main" | sudo tee /etc/apt/sources.list.d/llvm-17.list
curl https://apt.llvm.org/llvm-snapshot.gpg.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/llvm.gpg > /dev/null
# Ensure STL version
sudo apt-get install -y libstdc++-13-dev
```

Ensure cmake >3.24 is installed.

## Build the Tests

Some dependencies are not on Conan Center but only on our own Package repository. Add it with:
```shell
conan remote add dice-group https://conan.dice-research.org/artifactory/api/conan/tentris
```

Download `conan_provider.cmake` to the project directory:

```shell
wget https://github.com/conan-io/cmake-conan/raw/develop2/conan_provider.cmake -O conan_provider.cmake
```

(Optional) Some tests require libtorch. You can download it from https://pytorch.org/get-started/locally/ (works at
least with Stable|Linux|LibTorch|C++|None). We tested with version 10.9.0. There have been reports about some newer
versions being
broken.

Configure CMake (replace `path/to/libtorch` or remove the line before excuting).

```shell
cmake \
-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES="conan_provider.cmake" \
-DBUILD_TESTING=On \
-DLIBTORCH_PATH="path/to/libtorch" \
-B build .
```

Build:

```shell
cmake --build . --parallel
```

Run:

```shell
ctest --parallel
```

If you want to exclude long-running validation tests, run:

```shell
ctest --parallel --exclude-regex "(tests_RawHypertrieContext_systematic)|(tests_RawHypertrieContext_systematic_metall)|(tests_HypertrieContext_systematic_metall)|(tests_Einsum)|(tests_Einsum_metall)"
```

# running tests

To enable test, set `DBUILD_TESTING` in cmake:

```shell script
cmake -DBUILD_TESTING=ON ..
make -j tests
tests/tests
```

