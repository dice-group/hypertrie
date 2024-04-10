add_library(einsum INTERFACE)
add_library(hypertrie::einsum ALIAS einsum)

# for some reason it seems to be necessary to use find_package for all dependencies
# of einsum here again even though they are already found in hypertrie' CMakeLists.txt
# Otherwise, using einsum in the tests within target_link_libraries won't find its
# transitive dependencies.
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS)
find_package(robin_hood REQUIRED)
find_package(dice-sparse-map REQUIRED)
find_package(dice-hash REQUIRED)
find_package(dice-template-library REQUIRED)

target_link_libraries(einsum INTERFACE
        hypertrie::hypertrie
        )

add_dependencies(einsum hypertrie)

target_include_directories(einsum INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

include(${CMAKE_SOURCE_DIR}/cmake/install_interface_library.cmake)
install_interface_component(einsum src)
