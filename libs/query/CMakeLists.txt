add_library(query INTERFACE)
add_library(hypertrie::query ALIAS query)

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

target_link_libraries(query INTERFACE
        hypertrie::hypertrie
        )

add_dependencies(query hypertrie)

target_include_directories(query INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

include(${CMAKE_SOURCE_DIR}/cmake/install_interface_library.cmake)
install_interface_component(query src)
