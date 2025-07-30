cmake_minimum_required(VERSION 3.24)
project(hypertrie VERSION 0.10.0
        DESCRIPTION "The hypertrie is powering the Tentris triple store.")

include(cmake/boilerplate_init.cmake)
boilerplate_init()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/libs/hypertrie/src/dice/hypertrie/Hypertrie_version.hpp)

if (PROJECT_IS_TOP_LEVEL)
    set(CONAN_INSTALL_ARGS "${CONAN_INSTALL_ARGS};-o=boost/*:header_only=True")

    if (BUILD_TESTING)
        set(CONAN_INSTALL_ARGS "${CONAN_INSTALL_ARGS};-o=&:with_test_deps=True")
    endif ()
endif ()

set(style_files
        .clang-format
        .clang-tidy
)
foreach(style_file ${style_files})
    file(DOWNLOAD "https://raw.githubusercontent.com/dice-group/tentris-cpp-coding-guidelines/main/${style_file}"
            "${CMAKE_SOURCE_DIR}/${style_file}"
            TLS_VERIFY ON)
endforeach()

if (PROJECT_IS_TOP_LEVEL AND USE_CLANG_TIDY)
    include(cmake/ClangTidy.cmake)
endif ()

add_subdirectory(libs)

if (PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()
