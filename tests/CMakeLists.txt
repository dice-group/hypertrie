cmake_minimum_required(VERSION 3.13)

# code coverage
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if ("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
        message("Building with llvm Code Coverage Tools")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")

    elseif (CMAKE_COMPILER_IS_GNUCXX)
        message("Building with lcov Code Coverage Tools")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    endif ()
endif ()

find_package(Catch2 REQUIRED CONFIG)
add_executable(tests_hypertrie_internal node_based/NodeBasedTests.cpp)
set_target_properties(tests_hypertrie_internal PROPERTIES LINKER_LANGUAGE CXX)
add_executable(tests_einsum TestEinsum.cpp)
add_executable(test_left_join Tests.cpp)
target_link_libraries(tests_hypertrie_internal PRIVATE
        Catch2::Catch2
        hypertrie::hypertrie
        )
target_link_libraries(tests_einsum PRIVATE
        Catch2::Catch2
        hypertrie::hypertrie
        )
target_link_libraries(test_left_join PRIVATE
        Catch2::Catch2
        hypertrie::hypertrie
        )

set(hypertrie_LIBTORCH_PATH "" CACHE PATH "The installation directory of pytorch.")
if (hypertrie_LIBTORCH_PATH)
    # add path
    list(APPEND CMAKE_PREFIX_PATH ${hypertrie_LIBTORCH_PATH})
    find_package(Torch REQUIRED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
    target_link_libraries(tests_einsum PRIVATE
            ${TORCH_LIBRARIES}
            )
    # define C++ preprocess flag to include the tests depending on pytorch
    add_compile_definitions(HYPERTRIE_ENABLE_LIBTORCH)
endif ()

# detect the tests

include(CTest)
include(Catch)
catch_discover_tests(tests_hypertrie_internal)
catch_discover_tests(tests_einsum)

# copy files for testing to the binary folder
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

