add_library(einsum INTERFACE)

set_target_properties(einsum PROPERTIES CXX_STANDARD 20)

target_include_directories(einsum INTERFACE
        src)

target_link_libraries(einsum INTERFACE
        hypertrie
        )