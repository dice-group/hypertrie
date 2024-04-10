cmake_minimum_required(VERSION 3.16)
project(hypertrie
        VERSION 0.9.3
        DESCRIPTION "A flexible data structure for low-rank, sparse tensors supporting slices by any dimension and einstein summation (einsum) and a flexible query interface"
        )

include(cmake/boilerplate_init.cmake)
boilerplate_init()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/libs/hypertrie/src/dice/hypertrie/Hypertrie_version.hpp)

option(CONAN_CMAKE "If this should use conan cmake to fetch dependencies" On)
if (IS_TOP_LEVEL AND CONAN_CMAKE)
    include(cmake/conan_cmake.cmake)
    install_packages_via_conan(${CMAKE_CURRENT_SOURCE_DIR}/conanfile.py)
endif ()

if (IS_TOP_LEVEL AND USE_CLANG_TIDY)
    include(cmake/ClangTidy.cmake)
endif ()

add_subdirectory(libs)

if (IS_TOP_LEVEL AND BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()
