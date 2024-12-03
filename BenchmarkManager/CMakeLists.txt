# 设置源文件
set(BENCHMARK_SOURCES
    BenchmarkTool.cpp
    RunBenchmark.cpp
)

# 设置输出目录
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/BenchmarkManager/output)

# 确保输出目录存在
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# 添加可执行文件
add_executable(RunBenchmark ${BENCHMARK_SOURCES})

# 链接 cpp 库
target_include_directories(RunBenchmark PRIVATE ${PROJECT_SOURCE_DIR}/cpp)
target_link_libraries(RunBenchmark PRIVATE cpp_lib)