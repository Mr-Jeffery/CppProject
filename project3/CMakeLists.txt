cmake_minimum_required(VERSION 3.12)

# source /opt/intel/oneapi/compiler/2024.1/env/vars.sh
set(CMAKE_C_COMPILER "icx")

# add definitions, compiler flags, etc.
# add_definitions(-DWITH_AVX2)
# add_definitions(-mavx2)
# add_definitions(-mfma)
# add_definitions(-Ofast)
# add_definitions(-fopenmp)
# add_definitions(-march=native)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mavx2")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfma")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O4")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
set(CMAKE_C_STANDARD 17)

project(matmul)

add_executable(matmul_improved src/test.c)
add_executable(matmul matmul.c)

target_include_directories(matmul PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/build)

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    message("OpenMP found")
    target_link_libraries(matmul PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(matmul_improved PUBLIC OpenMP::OpenMP_CXX)
endif()
