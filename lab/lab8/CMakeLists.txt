cmake_minimum_required(VERSION 3.12)

# add definitions, compiler flags, etc.
add_definitions(-DWITH_AVX2)
add_definitions(-mavx2)
add_definitions(-O3)

set(CMAKE_CXX_STANDARD 17)

project(VectorAddition)

add_executable(VectorAddition vecadd.c)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message("OpenMP found")
    target_link_libraries(VectorAddition PUBLIC OpenMP::OpenMP_CXX)
endif()