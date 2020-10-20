cmake_minimum_required(VERSION 3.13)
project(hypertrie VERSION 0.5.2)
set(CMAKE_CXX_STANDARD 20)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "")
    message(" [INFO] VCPKG CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
endif()

if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer -momit-leaf-frame-pointer")
else ()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer")
endif ()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -O0")

# installation directories
set(hypertrie_INSTALL_INCLUDE_DIR "include" CACHE STRING "The installation include directory")
set(hypertrie_INSTALL_CMAKE_DIR "share/hypertrie/cmake" CACHE STRING "The installation cmake directory")

set(THREADS_PREFER_PTHREAD_FLAG)
find_package(Threads REQUIRED)

find_package(absl CONFIG REQUIRED)
find_package(tsl-hopscotch-map CONFIG REQUIRED)
find_package(tsl-sparse-map CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(cppitertools CONFIG REQUIRED)

find_path(ROBIN_HOOD_HASHING_INCLUDE_DIRS "robin_hood.h")
add_library(robin-hood-hashing INTERFACE)
add_library(robin-hood-hashing::robin-hood-hashing ALIAS robin-hood-hashing)
target_include_directories(robin-hood-hashing INTERFACE
        $<BUILD_INTERFACE:${ROBIN_HOOD_HASHING_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:include>
        )

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
        cppitertools::cppitertools
        absl::hash
        tsl::hopscotch_map
        fmt::fmt
        Boost::headers
        Threads::Threads
        tsl::hopscotch_map
        robin-hood-hashing
        )

if (HYPERTRIE_ENABLE_TOOLS)
add_executable(ids2hypertrie tools/IDs2Hypertrie.cpp)

target_link_libraries(ids2hypertrie
        hypertrie)
endif()

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
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hypertrie-config-version.cmake DESTINATION ${hypertrie_INSTALL_CMAKE_DIR})
install(DIRECTORY include/ DESTINATION ${hypertrie_INSTALL_INCLUDE_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hypertrie-config-version.cmake DESTINATION ${hypertrie_INSTALL_CMAKE_DIR})
install(DIRECTORY thirdparty/cppitertools/ DESTINATION ${hypertrie_INSTALL_INCLUDE_DIR})
