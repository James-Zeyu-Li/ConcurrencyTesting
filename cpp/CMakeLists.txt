# 设置源文件
set(CPP_SOURCES
    TaskQueue.cpp
    CSVHandler.cpp
    ProducerConsumerConcurrentIO.cpp
    util/MutexLock.cpp
    util/RWLock.cpp
    util/ThreadManager.cpp
)

# 创建一个静态库
add_library(cpp_lib STATIC ${CPP_SOURCES})

# 包含头文件目录
target_include_directories(cpp_lib PRIVATE ${PROJECT_SOURCE_DIR}/cpp)