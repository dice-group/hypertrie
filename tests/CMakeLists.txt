# code coverage
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
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

FetchContent_Declare(
        DocTest
        GIT_REPOSITORY "https://github.com/onqtam/doctest"
        GIT_TAG "2.4.6"
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(DocTest)

add_library(tests-utils INTERFACE)
target_include_directories(tests-utils INTERFACE
        utils)
target_link_libraries(tests-utils INTERFACE
        hypertrie-core)

add_executable(tests_SingleEntryNode core/node/tests_SingleEntryNode.cpp)
target_link_libraries(tests_SingleEntryNode
        doctest
        hypertrie-core
        hypertrie-core-fmt
        tests-utils
        fmt::fmt
        )

add_executable(tests_FullNode core/node/tests_FullNode.cpp)
target_link_libraries(tests_FullNode
        doctest
        hypertrie-core
        hypertrie-core-fmt
        tests-utils
        fmt::fmt
        )

add_executable(tests_AllocateNode core/node/tests_AllocateNode.cpp)
target_link_libraries(tests_AllocateNode
        doctest
        hypertrie-core
        hypertrie-core-fmt
        tests-utils
        fmt::fmt
        )

add_executable(tests_RawHypertrieContext core/node/tests_RawHypertrieContext.cpp)
target_link_libraries(tests_RawHypertrieContext
        doctest
        hypertrie-core
        fmt-definitions
        tests-utils
        fmt::fmt
        )

add_executable(tests_RawHypertrieContext_slice core/node/tests_RawHypertrieContext_slice.cpp)
target_link_libraries(tests_RawHypertrieContext_slice
        doctest
        hypertrie-core
        fmt-definitions
        tests-utils
        fmt::fmt
        )

add_executable(tests_RawHypertrieContext_randomized core/node/tests_RawHypertrieContext_randomized.cpp)
target_link_libraries(tests_RawHypertrieContext_randomized
        doctest
        hypertrie-core
        fmt-definitions
        tests-utils
        fmt::fmt
        )

add_executable(tests_RawHypertrieContext_randomized_large_data core/node/tests_RawHypertrieContext_randomized_large_data.cpp)
target_link_libraries(tests_RawHypertrieContext_randomized_large_data
        doctest
        hypertrie-core
        fmt-definitions
        tests-utils
        fmt::fmt
        )

add_executable(tests_RawHypertrieContext_systematic core/node/tests_RawHypertrieContext_systematic.cpp)
target_link_libraries(tests_RawHypertrieContext_systematic
        doctest
        hypertrie-core
        fmt-definitions
        tests-utils
        fmt::fmt
        )

add_executable(tests_Identifier core/node/tests_Identifier.cpp)
target_link_libraries(tests_Identifier
        doctest
        hypertrie-core
        hypertrie-core-fmt
        tests-utils
        fmt::fmt
        )

add_executable(tests_HypertrieContext hypertrie/tests_HypertrieContext.cpp)
target_link_libraries(tests_HypertrieContext
        doctest
        hypertrie
        tests-utils
        fmt::fmt
        )

add_executable(tests_Einsum einsum/tests_Einsum.cpp)
target_link_libraries(tests_Einsum
        doctest
        einsum
        tests-utils
        fmt::fmt
        )

add_executable(tests_RawHashDiagonal core/node/tests_RawHashDiagonal.cpp)
target_link_libraries(tests_RawHashDiagonal
        doctest
        hypertrie
        tests-utils
        fmt::fmt
        fmt-definitions
        )

add_executable(tests_RawIterator core/node/tests_RawIterator.cpp)
target_link_libraries(tests_RawIterator
        doctest
        hypertrie
        tests-utils
        fmt::fmt
        fmt-definitions
        )


# fmt
add_executable(tests_fmt_definitions fmt/tests_fmt_definitions.cpp)
target_link_libraries(tests_fmt_definitions
        doctest
        fmt-definitions
        )
add_executable(tests_einsum TestEinsum.cpp)
target_link_libraries(tests_einsum PRIVATE
        doctest
        einsum
        fmt::fmt
        tests-utils
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

# copy files for testing to the binary folder
#file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

