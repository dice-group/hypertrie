add_library(hypertrie-test-utils INTERFACE)

target_include_directories(hypertrie-test-utils INTERFACE
        .)

target_link_libraries(hypertrie-test-utils INTERFACE
        hypertrie
        fmt::fmt
        cppitertools::cppitertools
        )