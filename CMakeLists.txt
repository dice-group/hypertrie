cmake_minimum_required(VERSION 3.21)
project(hypertrie
        VERSION 0.15.0
        DESCRIPTION "A flexible data structure for low-rank, sparse tensors supporting slices by any dimension and einstein summation (einsum) and a flexible query interface"
        )

include(cmake/boilerplate_init.cmake)
boilerplate_init()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/libs/hypertrie/src/dice/hypertrie/Hypertrie_version.hpp)

option(BUILD_FFI "Build hypertrie foreign function interface" OFF)

option(USE_CONAN "If available, use conan to retrieve dependencies." ON)
if (PROJECT_IS_TOP_LEVEL AND USE_CONAN)
    include(cmake/conan_cmake.cmake)
    if (PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
        set(CONAN_HYPERTRIE_WITH_TEST_DEPS "True")
    else()
        set(CONAN_HYPERTRIE_WITH_TEST_DEPS "False")
    endif()

    if (BUILD_FFI)
        set(CONAN_HYPERTRIE_BUILD_FFI "True")
    else ()
        set(CONAN_HYPERTRIE_BUILD_FFI "False")
    endif ()

    set(CONAN_OPTIONS "with_test_deps=${CONAN_HYPERTRIE_WITH_TEST_DEPS};build_ffi=${CONAN_HYPERTRIE_BUILD_FFI};boost:header_only=True")
    install_packages_via_conan("${CMAKE_SOURCE_DIR}/conanfile.py" "${CONAN_OPTIONS}")
endif ()

if (PROJECT_IS_TOP_LEVEL AND USE_CLANG_TIDY)
    include(cmake/ClangTidy.cmake)
endif ()

add_subdirectory(libs)

if (PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()
