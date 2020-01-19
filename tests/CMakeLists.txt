cmake_minimum_required(VERSION 3.13)

# add the exectutable for all tests

find_package(Catch2 REQUIRED CONFIG)
find_package(Torch REQUIRED)


add_executable(tests Tests.cpp)
target_link_libraries(tests Catch2::Catch2
                            ${libraries_to_link}
                            hypertrie::hypertrie
                            ${TORCH_LIBRARIES}
        )
# detect the tests

include(CTest)
include(Catch)
catch_discover_tests(tests)

# copy files for testing to the binary folder
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

