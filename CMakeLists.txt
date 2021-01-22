cmake_minimum_required(VERSION 3.13)
project(hypertrie VERSION 0.6.0)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

# robin_hood hashing builds only with this approach :/
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -O0")

set(THREADS_PREFER_PTHREAD_FLAG)
find_package(Threads REQUIRED)


# installation directories
#todo: use absolute paths here with some decent prefix like ${CMAKE_BINARY_DIR}
set(hypertrie_INSTALL_INCLUDE_DIR "include" CACHE STRING "The installation include directory")
set(hypertrie_INSTALL_CMAKE_DIR "share/hypertrie/cmake" CACHE STRING "The installation cmake directory")

include(FetchContent)
FetchContent_Declare(
        cppitertools
        GIT_REPOSITORY https://github.com/ryanhaining/cppitertools.git
        GIT_TAG v2.1
        GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(cppitertools)

find_package(tsl-hopscotch-map REQUIRED)
find_package(tsl-sparse-map REQUIRED)
find_package(fmt REQUIRED)
find_package(dice-hash REQUIRED)

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
        tsl-hopscotch-map::tsl-hopscotch-map
        fmt::fmt
        Boost::Boost
        Threads::Threads
        tsl-sparse-map::tsl-sparse-map
        CONAN_PKG::robin-hood-hashing
        dice-hash::dice-hash
        )

if (HYPERTRIE_ENABLE_TOOLS)
add_executable(ids2hypertrie tools/IDs2Hypertrie.cpp)

target_link_libraries(ids2hypertrie
        hypertrie)
endif()

# testing
option(hypertrie_BUILD_TESTS "Build test programs" OFF)
if (hypertrie_BUILD_TESTS)
#    set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
    enable_testing()
    add_subdirectory(tests)
endif ()

# Make package findable
configure_file(cmake/dummy-config.cmake.in hypertrie-config.cmake @ONLY)

# Enable version checks in find_package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(hypertrie-config-version.cmake COMPATIBILITY SameMajorVersion)

# install and export target
install(TARGETS hypertrie EXPORT hypertrie-targets)

# hacky hack to make it build with itertools without exposing them
target_include_directories(hypertrie INTERFACE
        $<INSTALL_INTERFACE:${cppitertools_INSTALL_INCLUDE_DIR}/cppitertools>)

install(EXPORT hypertrie-targets
        FILE hypertrie-config.cmake
        NAMESPACE Dice::
        DESTINATION ${hypertrie_INSTALL_CMAKE_DIR}
        )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hypertrie-config-version.cmake DESTINATION ${hypertrie_INSTALL_CMAKE_DIR})
install(DIRECTORY include/ DESTINATION ${hypertrie_INSTALL_INCLUDE_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hypertrie-config-version.cmake DESTINATION ${hypertrie_INSTALL_CMAKE_DIR})
