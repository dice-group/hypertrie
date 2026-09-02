
> **⚠️ Note:** This is **not** the commercial version of Hypertrie, but a research prototype that focuses on aspects presented in papers. For a reliable, SPARQL feature-complete, and well-tested edition, see the commercial edition at https://github.com/tentris/tentris.

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
