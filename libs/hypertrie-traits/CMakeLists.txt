add_library(hypertrie-traits INTERFACE)

set_target_properties(hypertrie-traits PROPERTIES CXX_STANDARD 20)


target_include_directories(hypertrie-traits INTERFACE
        src
        )

target_link_libraries(hypertrie-traits
        INTERFACE
        dice-hash::dice-hash
        tsl::sparse_map)