add_library(hypertrie-core-fmt INTERFACE)

set_target_properties(hypertrie-core-fmt PROPERTIES CXX_STANDARD 20)

target_include_directories(hypertrie-core-fmt INTERFACE
        src)

target_link_libraries(hypertrie-core-fmt INTERFACE
        hypertrie-core
        Boost::boost
        fmt::fmt
        )
