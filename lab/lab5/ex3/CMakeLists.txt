cmake_minimum_required(VERSION 3.10)
project(StuScore)

set(CMAKE_CXX_STANDARD 14)

add_executable(StuScore main.cpp stufun.cpp)

target_include_directories(StuScore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/build)