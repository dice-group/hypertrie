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

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.13.4
        GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(Catch2)

# We are slowly migrating to DocTest. For some time we will use both test frameworks
FetchContent_Declare(
        DocTest
        GIT_REPOSITORY "https://github.com/onqtam/doctest"
        GIT_TAG  "2.4.6"
)
FetchContent_MakeAvailable(DocTest)

add_executable(tests_hypertrie_internal node_based/NodeBasedTests.cpp)
#set_target_properties(tests_hypertrie_internal PROPERTIES LINKER_LANGUAGE CXX)
add_executable(tests_einsum TestEinsum.cpp)
add_executable(tests_allocator_awareness allocator_awareness/TestAllocatorAwareness.cpp)
target_include_directories(tests_allocator_awareness PUBLIC allocator_awareness)
target_link_libraries(tests_hypertrie_internal PRIVATE
        Catch2::Catch2
        hypertrie::hypertrie
        )
target_link_libraries(tests_einsum PRIVATE
        Catch2::Catch2
        hypertrie::hypertrie
        )
target_link_libraries(tests_allocator_awareness PRIVATE
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

add_executable(tests_raw_diagonal node_based/TestRawDiagonal.cpp)
target_link_libraries(tests_raw_diagonal
        doctest
        hypertrie::hypertrie
        )

# detect the tests
include(CTest)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(Catch)
catch_discover_tests(tests_hypertrie_internal)
catch_discover_tests(tests_einsum)
catch_discover_tests(tests_allocator_awareness)

# copy files for testing to the binary folder
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

