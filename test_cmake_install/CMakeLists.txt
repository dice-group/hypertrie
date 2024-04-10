cmake_minimum_required(VERSION 3.16)
project(test_cmake_install VERSION 0.8.2)


list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../installed/")

include(${CMAKE_SOURCE_DIR}/../cmake/conan_cmake.cmake)
install_packages_via_conan("${CMAKE_SOURCE_DIR}/conanfile.txt")

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../installed/")

find_package(Boost REQUIRED)
find_package(dice-hash REQUIRED)
find_package(dice-sparse-map REQUIRED)
find_package(robin_hood REQUIRED)
find_package(hypertrie REQUIRED COMPONENTS einsum) # hypertrie is added automatically, einsum can be added additionally

find_package(Threads REQUIRED)

add_executable(example example.cpp)

target_link_libraries(example PUBLIC
        hypertrie::hypertrie
        hypertrie::einsum
        )

set_target_properties(
        example PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
