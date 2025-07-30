cmake_minimum_required(VERSION 3.21)
set(lib_suffix "query")
set(lib "${PROJECT_NAME}-${lib_suffix}")

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS)
find_package(robin_hood REQUIRED)
find_package(dice-sparse-map REQUIRED)
find_package(dice-hash REQUIRED)
find_package(dice-template-library REQUIRED)

add_library(${lib} INTERFACE)
add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

target_include_directories(${lib} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_link_libraries(${lib} INTERFACE
        ${PROJECT_NAME}::hypertrie
)

add_dependencies(${lib} hypertrie)

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(INTERFACE ${lib_suffix} src)
