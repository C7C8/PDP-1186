cmake_minimum_required(VERSION 3.5.1)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
project(PDP-1186)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror")

add_executable(PDP-1186 src/main.cpp)
add_executable(PDP-1186_test test/test.cpp)
target_link_libraries(PDP-1186_test gtest gtest_main)
