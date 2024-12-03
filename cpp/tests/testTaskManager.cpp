#include "../ProducerConsumerConcurrentIO.h"
#include "../TaskQueue.h"
#include "../util/MutexLock.h"
#include "../util/RWLock.h"
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>
using namespace std;

// Function to print section headers
void printSection(const std::string &sectionName) {
  std::cout << "=== " << sectionName << " ===" << std::endl;
}

void testMultipleConsumers() {
  TaskQueue queue(LockType::Mutex);
  ProducerConsumerConcurrentIO manager("test_multi.csv", queue);

  // multi   consumers
  manager.customTasks(3, 30, 0, 3, 30);

  std::this_thread::sleep_for(std::chrono::seconds(10));

  auto rows = manager.getCSVContent();
  assert(rows.size() == 30);
  cout << "Multiple consumers test passed.\n" << endl;
}

void testMultipleProducersConsumersWithCustomTasks() {
  printSection("Test: Multiple Producers and Consumers (CustomTasks)");

  TaskQueue queue(LockType::RWLock); // use RWLock
  ProducerConsumerConcurrentIO manager("test_rwlock.csv", queue);

  int producerThreads = 5;
  std::size_t produceCount = 50;
  int readerThreads = 2;
  int consumerThreads = 5;

  manager.customTasks(producerThreads, produceCount, readerThreads,
                      consumerThreads, produceCount);

  auto rows = manager.getCSVContent();
  assert(rows.size() == produceCount);

  cout << "CSV Content: " << endl;
  for (const auto &row : rows) {
    for (const auto &col : row) {
      cout << col << " ";
    }
    cout << endl;
  }

  cout << "Verified CSV content for RWLock successfully." << endl;
}

void testStressWithCustomTasks() {
  printSection("Test: Stress Test (CustomTasks)");

  TaskQueue queue(LockType::Mutex); // 使用互斥锁
  ProducerConsumerConcurrentIO manager("test_stress.csv", queue);

  // 高负载测试
  int producerThreads = 10;        // 10 个生产者线程
  std::size_t produceCount = 1000; // 总共生成 1000 个任务
  int readerThreads = 4;           // 1 个读取器线程
  int consumerThreads = 10;        // 10 个消费者线程

  manager.customTasks(producerThreads, produceCount, readerThreads,
                      consumerThreads, produceCount);

  // 等待所有任务完成
  std::this_thread::sleep_for(std::chrono::seconds(10));

  // 获取队列和文件的状态
  int queueSize = queue.queueSize();
  auto rows = manager.getCSVContent();

  // 断言检查
  assert(queueSize == 0);              // 队列应为空
  assert(rows.size() == produceCount); // 文件记录数应正确

  cout << "Queue size after processing: " << queueSize << endl;
  cout << "CSV file contains " << rows.size() << " rows." << endl;
  cout << "Verified CSV content under high load successfully." << endl;
}
void testReaderThreadWithOutput() {
  printSection("Test: Reader Thread With Output");

  // 初始化任务队列和管理器
  TaskQueue queue(LockType::Mutex);
  ProducerConsumerConcurrentIO manager("test_reader.csv", queue);

  // 准备一些任务并写入 CSV 文件
  auto csvHandler =
      std::make_unique<CSVHandler>("test_reader.csv", LockType::Mutex);
  std::vector<std::vector<std::string>> testData = {
      {"1", "Task_1", "Incomplete"},
      {"2", "Task_2", "Incomplete"},
      {"3", "Task_3", "Complete"}};
  for (const auto &row : testData) {
    csvHandler->writeRow(row);
  }

  // 启动 Reader 线程
  manager.startReaderThread();

  // 使用一个标志检查内容是否读取完成
  bool allTasksRead = false;
  while (!allTasksRead) {
    try {
      auto rows = csvHandler->readAll();
      std::cout << "Reader Thread Read Content:" << std::endl;
      for (const auto &row : rows) {
        for (const auto &cell : row) {
          std::cout << cell << " ";
        }
        std::cout << std::endl;
      }

      if (rows.size() >= testData.size()) {
        allTasksRead = true;
        std::cout << "All tasks read successfully by reader thread."
                  << std::endl;
      }
    } catch (const std::exception &e) {
      std::cerr << "Error during reading: " << e.what() << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // 停止 Reader 线程
  manager.stopReaderThread();

  // 验证读取的内容是否正确
  auto rows = csvHandler->readAll();
  assert(rows.size() == testData.size());
  for (size_t i = 0; i < testData.size(); ++i) {
    assert(rows[i] == testData[i]);
  }

  std::cout << "Reader thread test passed successfully with output."
            << std::endl;
}

// Test all cases
void comprehensiveTest() {

  // testMultipleConsumers();
  // testMultipleProducersConsumersWithCustomTasks();
  // testStressWithCustomTasks();
  testReaderThreadWithOutput();
  cout << "All tests passed successfully!" << endl;
}

int main() {
  try {
    comprehensiveTest();
  } catch (const exception &e) {
    cerr << "Test failed: " << e.what() << endl;
    return 1;
  }
  return 0;
}

// g++ -std=c++17 -Wall -Wextra -o testTaskManager \
//     ../TaskQueue.cpp \
//     ../ProducerConsumerConcurrentIO.cpp \
//     ../CSVHandler.cpp \
//     ../util/MutexLock.cpp \
//     ../util/RWLock.cpp \
//     ../util/ThreadManager.cpp \
//     testTaskManager.cpp -pthread