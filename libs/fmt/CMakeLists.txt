add_library(fmt-definitions INTERFACE)

set_target_properties(fmt-definitions PROPERTIES CXX_STANDARD 20)

target_include_directories(fmt-definitions INTERFACE
        src)

target_link_libraries(fmt-definitions INTERFACE
        hypertrie-traits
        hypertrie-core
        Boost::boost
        fmt::fmt
        )