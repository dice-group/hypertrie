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

#set(hypertrie_LIBTORCH_PATH "" CACHE PATH "The installation directory of pytorch.")
#if (hypertrie_LIBTORCH_PATH)
#    # add path
#    list(APPEND CMAKE_PREFIX_PATH ${hypertrie_LIBTORCH_PATH})
#    find_package(Torch REQUIRED)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
#    target_link_libraries(tests_einsum PRIVATE
#            ${TORCH_LIBRARIES}
#            )
#    # define C++ preprocess flag to include the tests depending on pytorch
#    add_compile_definitions(HYPERTRIE_ENABLE_LIBTORCH)
#endif ()

# copy files for testing to the binary folder
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

