cmake_minimum_required(VERSION 3.10)
project(socket_comm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

execute_process(COMMAND git rev-parse --short=40 HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                RESULT_VARIABLE GIT_RESULT
                OUTPUT_VARIABLE GIT_COMMIT
                OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT GIT_RESULT EQUAL 0)
  set(GIT_COMMIT "UNKNOWN")
endif()

configure_file(${SRC_DIR}/git_version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/git_version.h
               @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(client ${SRC_DIR}/client.cpp)

install(TARGETS client DESTINATION bin)
