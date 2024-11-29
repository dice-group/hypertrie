add_library(hypertrie-fmt INTERFACE)

target_include_directories(hypertrie-fmt INTERFACE
        src)

SET(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS)
find_package(dice-template-library REQUIRED)

target_link_libraries(hypertrie-fmt INTERFACE
        hypertrie::hypertrie
        Boost::headers
        fmt::fmt
        dice-template-library::dice-template-library
        )