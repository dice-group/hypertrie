add_library(atomic_queue INTERFACE)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

set_target_properties(atomic_queue PROPERTIES CXX_STANDARD 20)

target_include_directories(atomic_queue INTERFACE
        include)

target_link_libraries(atomic_queue INTERFACE
        Threads::Threads
        atomic
        )

# TODO: replace with git submodule or cmake fetch_content_declare