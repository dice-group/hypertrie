cmake_minimum_required(VERSION 3.10.2)
project(hypertrie VERSION 0.5.2.11)
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer -momit-leaf-frame-pointer")

    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
else ()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer")
endif ()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -O0")

set(THREADS_PREFER_PTHREAD_FLAG)
find_package(Threads REQUIRED)

# installation directories
set(hypertrie_INSTALL_INCLUDE_DIR "include" CACHE STRING "The installation include directory")
set(hypertrie_INSTALL_CMAKE_DIR "share/hypertrie/cmake" CACHE STRING "The installation cmake directory")

#include cppitertools, see https://github.com/ryanhaining/cppitertools
add_library(cppitertools INTERFACE)
add_library(cppitertools::cppitertools ALIAS cppitertools)

target_include_directories(cppitertools INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppitertools/>
        $<INSTALL_INTERFACE:include>
        )

find_package(abseil REQUIRED)
find_package(tsl-hopscotch-map REQUIRED)
find_package(tsl-sparse-map REQUIRED)
find_package(fmt CONFIG REQUIRED)

SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS)

add_library(hypertrie INTERFACE)
add_library(hypertrie::hypertrie ALIAS hypertrie)

target_include_directories(hypertrie INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${hypertrie_INSTALL_CMAKE_DIR}>
        ${Boost_INCLUDE_DIRS}
        )


target_link_libraries(hypertrie INTERFACE
        cppitertools
        abseil::abseil
        tsl-hopscotch-map::tsl-hopscotch-map
        fmt::fmt
        Boost::boost
        Threads::Threads
        tsl-sparse-map::tsl-sparse-map
        )

# testing
option(hypertrie_BUILD_TESTS "Build test programs" OFF)
if (hypertrie_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

# Make package findable
configure_file(cmake/dummy-config.cmake.in hypertrie-config.cmake @ONLY)

# Enable version checks in find_package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(hypertrie-config-version.cmake COMPATIBILITY SameMajorVersion)

# install and export target
install(TARGETS hypertrie cppitertools EXPORT hypertrie-targets)

install(EXPORT hypertrie-targets
        FILE hypertrie-config.cmake
        NAMESPACE Dice::
        DESTINATION ${hypertrie_INSTALL_CMAKE_DIR}
        )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hypertrie-config-version.cmake DESTINATION ${hypertrie_INSTALL_CMAKE_DIR})
install(DIRECTORY include/ DESTINATION ${hypertrie_INSTALL_INCLUDE_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hypertrie-config-version.cmake DESTINATION ${hypertrie_INSTALL_CMAKE_DIR})
install(DIRECTORY thirdparty/cppitertools/ DESTINATION ${hypertrie_INSTALL_INCLUDE_DIR})
