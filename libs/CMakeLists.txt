cmake_minimum_required(VERSION 3.21)

add_subdirectory(hypertrie)
add_subdirectory(einsum)
add_subdirectory(query)

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_package()
