add_library(hypertrie INTERFACE)
add_library(hypertrie::hypertrie ALIAS hypertrie)

find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS)
find_package(robin_hood REQUIRED)
find_package(dice-sparse-map REQUIRED)
find_package(dice-hash REQUIRED)
find_package(dice-template-library REQUIRED)

target_link_libraries(hypertrie INTERFACE
        Threads::Threads
        Boost::headers
        robin_hood::robin_hood
        dice-hash::dice-hash
        dice-sparse-map::dice-sparse-map
        dice-template-library::dice-template-library
        )

target_include_directories(hypertrie INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

include(${CMAKE_SOURCE_DIR}/cmake/install_interface_library.cmake)
install_interface_component(hypertrie src)