cmake_minimum_required(VERSION 3.15)
project(kv_engine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kv_server
    main.cpp
    server.cpp
    store.cpp
    resp_parser.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(kv_server PRIVATE Threads::Threads)