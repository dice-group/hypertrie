set(CMAKE_CXX_STANDARD 20)
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

find_package(doctest REQUIRED)
find_package(cppitertools REQUIRED)
find_package(Metall REQUIRED)
find_package(Boost REQUIRED COMPONENTS)
find_package(fmt REQUIRED)
SET(Boost_USE_STATIC_LIBS ON)


add_subdirectory(libhypertrie-test-utils)
add_subdirectory(libhypertrie-fmt)

add_executable(tests_SingleEntryNode core/node/tests_SingleEntryNode.cpp)
target_link_libraries(tests_SingleEntryNode
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_SingleEntryNode COMMAND tests_SingleEntryNode)


add_executable(tests_FullNode core/node/tests_FullNode.cpp)
target_link_libraries(tests_FullNode
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_FullNode COMMAND tests_FullNode)


add_executable(tests_AllocateNode core/node/builds_NodeStorage.cpp)
target_link_libraries(tests_AllocateNode
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_AllocateNode COMMAND tests_AllocateNode)


add_executable(tests_RawHypertrieContext core/node/tests_RawHypertrieContext.cpp)
target_link_libraries(tests_RawHypertrieContext
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_RawHypertrieContext COMMAND tests_RawHypertrieContext)


add_executable(tests_RawHypertrieContext_slice core/node/tests_RawHypertrieContext_slice.cpp)
target_link_libraries(tests_RawHypertrieContext_slice
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_RawHypertrieContext_slice COMMAND tests_RawHypertrieContext_slice)


add_executable(tests_RawHypertrieContext_randomized core/node/tests_RawHypertrieContext_randomized.cpp)
target_link_libraries(tests_RawHypertrieContext_randomized
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_RawHypertrieContext_randomized COMMAND tests_RawHypertrieContext_randomized)


add_executable(tests_RawHypertrieContext_randomized_large_data core/node/tests_RawHypertrieContext_randomized_large_data.cpp)
target_link_libraries(tests_RawHypertrieContext_randomized_large_data
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_RawHypertrieContext_randomized_large_data COMMAND tests_RawHypertrieContext_randomized_large_data)


add_executable(tests_RawHypertrieContext_systematic core/node/tests_RawHypertrieContext_systematic.cpp)
target_link_libraries(tests_RawHypertrieContext_systematic
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_RawHypertrieContext_systematic COMMAND tests_RawHypertrieContext_systematic)

add_executable(tests_RawHypertrieContext_systematic_metall core/node/tests_RawHypertrieContext_systematic_metall.cpp)
target_link_libraries(tests_RawHypertrieContext_systematic_metall
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        Metall::Metall
        )
add_test(NAME tests_RawHypertrieContext_systematic_metall COMMAND tests_RawHypertrieContext_systematic_metall)


add_executable(tests_Identifier core/node/tests_Identifier.cpp)
target_link_libraries(tests_Identifier
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_Identifier COMMAND tests_Identifier)

add_executable(tests_Diagonal hypertrie/tests_Diagonal.cpp)
target_link_libraries(tests_Diagonal
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_Diagonal COMMAND tests_Diagonal)

add_executable(tests_HypertrieContext hypertrie/tests_HypertrieContext.cpp)
target_link_libraries(tests_HypertrieContext
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-test-utils
        fmt::fmt
        )
add_test(NAME tests_HypertrieContext COMMAND tests_HypertrieContext)

add_executable(tests_HypertrieContext_metall hypertrie/tests_HypertrieContext_metall.cpp)
target_link_libraries(tests_HypertrieContext_metall
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        Metall::Metall
        )
add_test(NAME tests_HypertrieContext_metall COMMAND tests_HypertrieContext_metall)

add_executable(tests_HypertrieContext_systematic_metall hypertrie/tests_HypertrieContext_systematic_metall.cpp)
target_link_libraries(tests_HypertrieContext_systematic_metall
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        Metall::Metall
        )
add_test(NAME tests_HypertrieContext_systematic_metall COMMAND tests_HypertrieContext_systematic_metall)

add_executable(tests_HypertrieIterator hypertrie/tests_HypertrieIterator.cpp)
target_link_libraries(tests_HypertrieIterator
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-fmt
        hypertrie-test-utils
        fmt::fmt
        Metall::Metall
        )
add_test(NAME tests_HypertrieIterator COMMAND tests_HypertrieIterator)

add_executable(tests_RawHashDiagonal core/node/tests_RawHashDiagonal.cpp)
target_link_libraries(tests_RawHashDiagonal
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-test-utils
        fmt::fmt
        hypertrie-fmt
        Metall::Metall
        )
add_test(NAME tests_RawHashDiagonal COMMAND tests_RawHashDiagonal)


add_executable(tests_RawIterator core/node/tests_RawIterator.cpp)
target_link_libraries(tests_RawIterator
        doctest::doctest
        hypertrie::hypertrie
        hypertrie-test-utils
        fmt::fmt
        hypertrie-fmt
        )
add_test(NAME tests_RawIterator COMMAND tests_RawIterator)


# fmt
add_executable(tests_fmt_definitions fmt/tests_fmt_definitions.cpp)
target_link_libraries(tests_fmt_definitions
        doctest::doctest
        hypertrie-fmt
        )
add_test(NAME tests_fmt_definitions COMMAND tests_fmt_definitions)


set(LIBTORCH_PATH "" CACHE PATH "The installation directory of pytorch.")
if (LIBTORCH_PATH)
    list(APPEND CMAKE_PREFIX_PATH ${LIBTORCH_PATH})

    find_package(Torch REQUIRED)
    add_executable(tests_Einsum einsum/tests_Einsum.cpp)
    target_link_libraries(tests_Einsum
            doctest::doctest
            hypertrie::einsum
            hypertrie-test-utils
            fmt::fmt
            ${TORCH_LIBRARIES}
            )
    add_dependencies(tests_Einsum hypertrie)

    target_compile_options(tests_Einsum PRIVATE "${TORCH_CXX_FLAGS}")

    add_test(NAME tests_Einsum COMMAND tests_Einsum)

    add_executable(tests_Einsum_metall einsum/tests_Einsum_metall.cpp)
    target_link_libraries(tests_Einsum_metall
            doctest::doctest
            hypertrie::einsum
            hypertrie-fmt
            hypertrie-test-utils
            fmt::fmt
            Metall::Metall
            ${TORCH_LIBRARIES}
            )

    target_compile_options(tests_Einsum_metall PRIVATE "${TORCH_CXX_FLAGS}")


    add_test(NAME tests_Einsum_metall COMMAND tests_Einsum_metall)

    add_executable(minimal_example einsum/minimal_example.cpp)
    target_link_libraries(minimal_example
            doctest::doctest
            hypertrie-test-utils
            Metall::Metall
            )
endif ()

add_executable(tests_OperandDependencyGraph query/tests_OperandDependencyGraph.cpp)
target_link_libraries(tests_OperandDependencyGraph
        doctest::doctest
        hypertrie::query
        )
add_test(NAME tests_OperandDependencyGraph COMMAND tests_OperandDependencyGraph)


add_executable(tests_Query query/tests_Query.cpp)
target_link_libraries(tests_Query
        doctest::doctest
        hypertrie::query
        )
add_test(NAME tests_Query COMMAND tests_Query)

# copy files for testing to the binary folder
#file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

