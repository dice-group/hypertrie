cmake_minimum_required(VERSION 3.21)

find_package(Metall REQUIRED)
find_package(metall-ffi REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/check_type_size.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/check_type_size.cpp @ONLY)
add_executable(check_type_size_bin check_type_size.cpp)

target_link_libraries(check_type_size_bin
        PRIVATE
        hypertrie::hypertrie
        hypertrie::einsum
        metall-ffi::metall-ffi
        )

target_include_directories(check_type_size_bin
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

set_target_properties(check_type_size_bin
        PROPERTIES
        CXX_STANDARD 20
        )

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generating-done.cpp
        COMMAND check_type_size_bin
        MAIN_DEPENDENCY src/dice/ffi/hypertrie.h.in
        DEPENDS check_type_size_bin
        VERBATIM)


add_library(hypertrie-ffi
        src/dice/ffi/hypertrie.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/generating-done.cpp
        )

add_library(hypertrie::ffi ALIAS hypertrie-ffi)

add_dependencies(hypertrie-ffi check_type_size_bin)

target_link_libraries(hypertrie-ffi
        PRIVATE
        hypertrie::hypertrie
        hypertrie::einsum
        PUBLIC
        metall-ffi::metall-ffi
        )

target_include_directories(hypertrie-ffi
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

set_target_properties(hypertrie-ffi PROPERTIES
        CXX_STANDARD 20
        POSITION_INDEPENDENT_CODE ON
        )

## Packaging and exporting the target
include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(PUBLIC hypertrie-ffi src)
