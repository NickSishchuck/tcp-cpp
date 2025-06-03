cmake_minimum_required(VERSION 3.20)
project(tcp_server_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

find_package(Threads REQUIRED)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE Threads::Threads)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE Threads::Threads)

# Optional: Set output directory
# set_target_properties(server client PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )
