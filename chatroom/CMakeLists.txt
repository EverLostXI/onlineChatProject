cmake_minimum_required(VERSION 3.10)
project(Chat LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 两个可执行文件
add_executable(chat_server server.cpp)
add_executable(chat_client client.cpp)

# 链接winsock库
target_link_libraries(
    chat_server
    ws2_32
)
target_link_libraries(
    chat_client
    ws2_32
)