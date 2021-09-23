add_library(hypertrie-core INTERFACE)

set_target_properties(hypertrie-core PROPERTIES CXX_STANDARD 20)

target_include_directories(hypertrie-core INTERFACE
        src)

target_link_libraries(hypertrie-core INTERFACE
        hypertrie-traits
        hypertrie-utils
        cppitertools::cppitertools
        )
