cmake_minimum_required(VERSION 3.21)
set(lib_suffix "query")
set(lib "${PROJECT_NAME}-${lib_suffix}")

add_library(${lib} INTERFACE)
add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

# for some reason it seems to be necessary to use find_package for all dependencies
# of query here again even though they are already found in hypertrie' CMakeLists.txt
# Otherwise, using query in the tests within target_link_libraries won't find its
# transitive dependencies.
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS)
find_package(robin_hood REQUIRED)
find_package(dice-sparse-map REQUIRED)
find_package(dice-hash REQUIRED)
find_package(dice-template-library REQUIRED)

target_link_libraries(${lib} INTERFACE
        hypertrie::hypertrie
        )

target_include_directories(${lib} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(INTERFACE ${lib_suffix} src)
