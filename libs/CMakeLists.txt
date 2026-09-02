cmake_minimum_required(VERSION 3.21)

add_subdirectory(hypertrie)
add_subdirectory(einsum)

if (BUILD_FFI)
    add_subdirectory(ffi)
endif ()

install_package()
