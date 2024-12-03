cmake_minimum_required(VERSION 3.10)
project(ConcurrencyTesting)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加子目录
add_subdirectory(cpp)
add_subdirectory(BenchmarkManager)

set(OUTPUT_DIR "${CMAKE_BINARY_DIR}/output")
file(MAKE_DIRECTORY ${OUTPUT_DIR})
add_definitions(-DOUTPUT_DIR="${OUTPUT_DIR}")