add_library(hypertrie-utils INTERFACE)

set_target_properties(hypertrie-utils PROPERTIES CXX_STANDARD 20)


target_include_directories(hypertrie-utils INTERFACE
        src
        )

target_link_libraries(hypertrie-utils
        INTERFACE
        hypertrie-traits
        )