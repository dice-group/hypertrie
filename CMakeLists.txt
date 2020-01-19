cmake_minimum_required(VERSION 3.12)
project(hypertrie VERSION 0.5.0)
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

# installation directories
set(hypertrie_INSTALL_INCLUDE_DIR "include" CACHE STRING "The installation include directory")
set(hypertrie_INSTALL_CMAKE_DIR "share/hypertrie/cmake" CACHE STRING "The installation cmake directory")

add_subdirectory(thirdparty/sparse-map)

#include cppitertools, see https://github.com/ryanhaining/cppitertools
add_library(cppitertools INTERFACE)
add_library(cppitertools::cppitertools ALIAS cppitertools)

target_include_directories(cppitertools INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/cppitertools/>
        $<INSTALL_INTERFACE:include>
        )

set(libraries_to_link
        cppitertools
        tsl::sparse_map
        ${CONAN_LIBS}
        )

add_library(hypertrie INTERFACE)
add_library(hypertrie::hypertrie ALIAS hypertrie)

target_include_directories(hypertrie INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${hypertrie_INSTALL_CMAKE_DIR}>
        )


target_link_libraries(hypertrie INTERFACE ${libraries_to_link}
        )


# require C++17
target_compile_features(hypertrie INTERFACE cxx_std_17)

# testing
option(hypertrie_BUILD_TESTS "Build test programs" OFF)
if(hypertrie_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

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