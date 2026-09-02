cmake_minimum_required(VERSION 3.21)

add_library(hypertrie INTERFACE)
add_library(hypertrie::hypertrie ALIAS hypertrie)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS)
find_package(dice-sparse-map REQUIRED)
find_package(dice-hash REQUIRED)
find_package(dice-template-library REQUIRED)
find_package(unordered_dense REQUIRED)

target_link_libraries(hypertrie INTERFACE
        Threads::Threads
        Boost::headers
        unordered_dense::unordered_dense
        dice-hash::dice-hash
        dice-sparse-map::dice-sparse-map
        dice-template-library::dice-template-library
        )

target_include_directories(hypertrie INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(INTERFACE hypertrie src)