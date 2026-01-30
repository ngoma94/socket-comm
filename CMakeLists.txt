cmake_minimum_required(VERSION 3.10)
project(socket_comm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(client ${SRC_DIR}/client.cpp)

install(TARGETS client DESTINATION bin)
