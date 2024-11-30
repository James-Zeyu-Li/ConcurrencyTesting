#include "../TaskQueue.h"
#include "../TaskThreadManager.h"
#include "../util/MutexLock.h"
#include "../util/RWLock.h"
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

using namespace std;

// Helper function to print test section
void printSection(const string &section) {
  cout << "\n===== " << section << " =====\n" << endl;
}

void testBasicFunctionalityWithMutex() {
  printSection("Test: Basic Functionality with MutexLock");

  TaskQueue queue(LockType::Mutex);
  TaskThreadManager manager("test_mutex.csv", queue);

  // Start producer, consumer, and reader threads
  manager.startProducerThread(10); // 10 tasks
  manager.startConsumerThread();
  manager.startReaderThread();

  // Wait for threads to process tasks
  this_thread::sleep_for(chrono::seconds(2));

  // Stop threads
  manager.stopConsumerThread(1); // Only one consumer
  manager.stopReaderThread();

  // Verify CSV content
  auto rows = manager.getCSVContent();
  assert(rows.size() == 10); // No header, only tasks
  cout << "Verified CSV content for MutexLock successfully." << endl;
}

void testSingleConsumer() {
  TaskQueue queue(LockType::Mutex);
  TaskThreadManager manager("test.csv", queue);

  // add tasks to the queue
  for (int i = 1; i <= 10; ++i) {
    queue.enqueue(Task{i, "Task_" + std::to_string(i), false});
  }

  // activate consumer thread
  manager.startConsumerThread();

  // wait for the consumer thread to finish
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // stop the consumer thread
  manager.stopConsumerThread(1);

  // check the CSV content
  auto rows = manager.getCSVContent();
  assert(rows.size() == 10);
  cout << "Single consumer test passed.\n" << endl;
}

void testMultipleConsumers() {
  TaskQueue queue(LockType::Mutex);
  TaskThreadManager manager("test_multi.csv", queue);

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
  TaskThreadManager manager("test_rwlock.csv", queue);

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
  TaskThreadManager manager("test_stress.csv", queue);

  // 高负载测试
  int producerThreads = 10;        // 10 个生产者线程
  std::size_t produceCount = 1000; // 总共生成 1000 个任务
  int readerThreads = 1;           // 1 个读取器线程
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
// Test all cases
void comprehensiveTest() {
  printSection("Comprehensive Test");
  // testBasicFunctionalityWithMutex();
  // testSingleConsumer();
  // testMultipleConsumers();
  // testMultipleProducersConsumersWithCustomTasks();
  testStressWithCustomTasks();

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
//     ../TaskThreadManager.cpp \
//     ../CSVHandler.cpp \
//     ../util/LockImpl/MutexLock.cpp \
//     ../util/LockImpl/RWLock.cpp \
//     ../util/ThreadManager.cpp \
//     testTaskManager.cpp -pthread