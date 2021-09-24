cmake_minimum_required(VERSION 3.13)
project(hypertrie VERSION 0.6.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
set(THREADS_PREFER_PTHREAD_FLAG)


include(CheckIPOSupported)
check_ipo_supported()
if (CMAKE_BUILD_TYPE MATCHES "Release")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # LTO
    add_link_options($<$<CONFIG:RELEASE>:-s>) #strip binaries
endif ()

### conan cmake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            EXPECTED_HASH SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
            TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_autodetect(settings)


conan_cmake_run(CONANFILE conanfile.py
        GENERATORS cmake_find_package
        BASIC_SETUP CMAKE_TARGETS
        BUILD missing)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

if (NOT EXISTS ${CMAKE_BINARY_DIR}/CMakeCache.txt)
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
    endif ()
endif ()

include(FetchContent)
FetchContent_Declare(
        cppitertools
        GIT_REPOSITORY https://github.com/ryanhaining/cppitertools.git
        GIT_TAG b2b98e60438f1ed6b04b77cdb6cc5d5516af301b
        GIT_SHALLOW TRUE)

FetchContent_Declare(
        tsl-sparse-map-our-version
        GIT_REPOSITORY https://github.com/dice-group/sparse-map.git
        GIT_TAG custom_tests
        GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(cppitertools)
FetchContent_MakeAvailable(tsl-sparse-map-our-version)

FetchContent_Declare(
        metall
        GIT_REPOSITORY https://github.com/LLNL/metall.git
        GIT_TAG         e53ff5992d00136052e1e903c9e4e93da1e25981 # v0.15
        GIT_SHALLOW    TRUE
)
FetchContent_GetProperties(metall)
FetchContent_Populate(metall)
include_directories(${metall_SOURCE_DIR}/include)

find_package(robin_hood REQUIRED)
find_package(tsl-hopscotch-map REQUIRED)
#find_package(tsl-sparse-map REQUIRED)
find_package(fmt REQUIRED)
find_package(dice-hash REQUIRED)

SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS)

#add_library(hypertrie INTERFACE)
#add_library(hypertrie::hypertrie ALIAS hypertrie)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/include/Dice/hypertrie/hypertrie_version.hpp)

add_subdirectory(libs)
add_subdirectory(tests)
#target_include_directories(hypertrie INTERFACE
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#        $<INSTALL_INTERFACE:${hypertrie_INSTALL_CMAKE_DIR}>
#        ${Boost_INCLUDE_DIRS}
#        )
#
#
#target_link_libraries(hypertrie INTERFACE
#        cppitertools::cppitertools
#        fmt::fmt
#        Boost::Boost
#        Threads::Threads
#        tsl::hopscotch_map
#        tsl::sparse_map
#        robin_hood::robin_hood
#        dice-hash::dice-hash
#        )

#if (HYPERTRIE_ENABLE_TOOLS)
#add_executable(ids2hypertrie tools/IDs2Hypertrie.cpp)
#
#target_link_libraries(ids2hypertrie
#        hypertrie)
#endif()
#
## testing
#option(hypertrie_BUILD_TESTS "Build test programs" OFF)
#if (hypertrie_BUILD_TESTS)
##    set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
#    enable_testing()
#    add_subdirectory(tests)
#endif ()
#
## Make package findable
#configure_file(cmake/dummy-config.cmake.in hypertrie-config.cmake @ONLY)
#
## Enable version checks in find_package
#include(CMakePackageConfigHelpers)
#write_basic_package_version_file(hypertrie-config-version.cmake COMPATIBILITY SameMajorVersion)
#
## install and export target
#install(TARGETS hypertrie EXPORT hypertrie-targets)
#
## hacky hack to make it build with itertools without exposing them
#target_include_directories(hypertrie INTERFACE
#        $<INSTALL_INTERFACE:${cppitertools_INSTALL_INCLUDE_DIR}/cppitertools>)
#
#install(EXPORT hypertrie-targets
#        FILE hypertrie-config.cmake
#        NAMESPACE Dice::
#        DESTINATION ${hypertrie_INSTALL_CMAKE_DIR}
#        )
#
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hypertrie-config-version.cmake DESTINATION ${hypertrie_INSTALL_CMAKE_DIR})
#install(DIRECTORY include/ DESTINATION ${hypertrie_INSTALL_INCLUDE_DIR})
#
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hypertrie-config-version.cmake DESTINATION ${hypertrie_INSTALL_CMAKE_DIR})
