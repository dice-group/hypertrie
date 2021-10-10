add_library(hypertrie INTERFACE)

set_target_properties(hypertrie PROPERTIES CXX_STANDARD 20)

target_include_directories(hypertrie INTERFACE
        src)

target_link_libraries(hypertrie INTERFACE
        hypertrie-core
        cppitertools::cppitertools
        )
