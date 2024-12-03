// main.cpp
#include "BenchmarkTool.h"
#include <filesystem>
#include <iostream>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

// 设置输出目录
void setupOutputDirectory(const string &outputPath) {
  if (!filesystem::exists(outputPath)) {
    filesystem::create_directories(outputPath);
    cout << "Created output directory: " << outputPath << endl;
  } else {
    cout << "Output directory already exists: " << outputPath << endl;
  }
}

// Test function for thread benchmark and runThreadBenchmark----------
void threadTestFunc(TaskQueue &taskQueue, int producerCount, int consumerCount,
                    int operationCount) {
  vector<thread> producers, consumers;
  atomic<int> tasksProduced{0}; // 跟踪生产的任务数
  atomic<int> tasksConsumed{0}; // 跟踪消费的任务数

  int tasksPerProducer = operationCount / producerCount;
  int remainingTasks = operationCount % producerCount; // 用于处理余数任务

  // 启动生产者线程
  for (int i = 0; i < producerCount; ++i) {
    producers.emplace_back(
        [&taskQueue, &tasksProduced, tasksPerProducer, remainingTasks, i]() {
          int tasksToProduce = tasksPerProducer + (i < remainingTasks ? 1 : 0);
          for (int j = 0; j < tasksToProduce; ++j) {
            int taskId = tasksProduced.fetch_add(1); // 获取全局任务 ID
            taskQueue.enqueue(Task{taskId, "Task_" + to_string(taskId), false});
            cout << "Producer_" << i << " produced Task_" << taskId << endl;
          }
        });
  }

  // 启动消费者线程
  for (int i = 0; i < consumerCount; ++i) {
    consumers.emplace_back(
        [&taskQueue, &tasksConsumed, &tasksProduced, operationCount, i]() {
          Task t;
          while (true) {
            if (taskQueue.dequeue(t)) {
              if (t.id == -1)
                break; // 检测到终止信号
              cout << "Consumer_" << i << " consumed Task_" << t.id << endl;
              tasksConsumed.fetch_add(1);
            }
            // 如果消费的任务数量已经达到总任务数，退出循环
            if (tasksConsumed.load() >= operationCount)
              break;
          }
        });
  }

  // 等待所有生产者线程完成
  for (auto &producer : producers) {
    producer.join();
  }

  // 添加终止信号，通知消费者退出
  for (int i = 0; i < consumerCount; ++i) {
    taskQueue.enqueue(Task{-1, "Terminate", true});
  }

  // 等待所有消费者线程完成
  for (auto &consumer : consumers) {
    consumer.join();
  }

  cout << "All tasks produced: " << tasksProduced.load() << endl;
  cout << "All tasks consumed: " << tasksConsumed.load() << endl;
}

// Thread benchmark, producer-consumer test
void runThreadBenchmark() {
  vector<string> lockTypes = {"MutexLock", "RWLock"};
  vector<pair<int, int>> threadConfigurations = {
      {1, 1}, // 1 producer, 1 consumer
      {4, 4}, // 4 producers, 4 consumers
      {8, 2}, // 8 producers, 2 consumers
      {2, 8}, // 2 producers, 8 consumers
  };
  vector<int> operationCounts = {10, 100, 1000, 10000};

  cout << "Running Producer-Consumer Thread Benchmark...\n" << endl;

  auto threadResults = BenchmarkTool::runThreadBenchmark(
      "Producer-Consumer Test", lockTypes, threadConfigurations,
      operationCounts, threadTestFunc);

  BenchmarkTool::exportThreadResultsToCSV("ResultThread.csv", threadResults);
}
// -------------------------------------------------------------------

// I/O benchmark test function and runIOBenchmark---------------------
void ioTestFunc(CSVHandler &csvHandler, int writerCount, int readerCount,
                int operationCount) {
  int operationsPerThread =
      operationCount / writerCount; // 每个线程完成的写入操作数
  if (operationCount % writerCount != 0) {
    cerr << "Operation count must be divisible by writer count for balanced "
            "distribution."
         << endl;
    return;
  }

  // 写线程
  vector<thread> writers;
  for (int i = 0; i < writerCount; ++i) {
    writers.emplace_back([&csvHandler, operationsPerThread, i]() {
      vector<string> rows;
      for (int j = 0; j < operationsPerThread; ++j) {
        rows.push_back("Writer_" + std::to_string(i) + ",Row_" +
                       std::to_string(j));
      }
      csvHandler.writeRow(rows); // 批量写入
    });
  }

  // 等待写线程完成
  for (auto &writer : writers) {
    writer.join();
  }

  cout << "All writers completed. Total rows written: " << operationCount
       << endl;

  // 读线程
  vector<thread> readers;
  for (int i = 0; i < readerCount; ++i) {
    readers.emplace_back([&csvHandler, i]() {
      auto rows = csvHandler.readAll();
      cout << "Reader_" << i << " read " << rows.size() << " rows." << endl;
    });
  }

  // 等待读线程完成
  for (auto &reader : readers) {
    reader.join();
  }

  // 清空 CSV 文件
  csvHandler.clear();
  cout << "CSV file cleared." << endl;
}

// IO benchmark test function
void runIOBenchmark() {
  vector<string> lockTypes = {"MutexLock", "RWLock"};
  vector<int> writerCounts = {1, 2, 5, 10};             // 增加 Writer 数量覆盖
  vector<int> readerCounts = {1, 2, 5, 10};             // 增加 Reader 数量覆盖
  vector<int> operationCounts = {10, 100, 1000, 10000}; // 不同负载覆盖

  cout << "Running I/O Benchmark...\n" << endl;
  auto ioResults =
      BenchmarkTool::runIOBenchmark("IO Test", lockTypes, writerCounts,
                                    readerCounts, operationCounts, ioTestFunc);

  BenchmarkTool::exportIOResultsToCSV("ResultIO.csv", ioResults);
}

// -------------------------------------------------------------------
//--
//--
//--
//--
//--
//--
//--
// Custom test
//     function-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
//     --void
// void customTestFunc(const std::string &lockType,
//                     std::shared_ptr<TaskQueue> taskQueue, int producerCount,
//                     int consumerCount, int readerCount, int operationCount) {
//   std::cout << "[customTestFunc] Starting with lockType: " << lockType
//             << ", producerCount: " << producerCount
//             << ", consumerCount: " << consumerCount
//             << ", readerCount: " << readerCount
//             << ", operationCount: " << operationCount << std::endl;

//   // Map lockType string to LockType enum
//   LockType lockTypeEnum =
//       (lockType == "MutexLock") ? LockType::Mutex : LockType::RWLock;

//   // Create and initialize I/O system with correct LockType
//   ProducerConsumerConcurrentIO ioSystem("custom_test.csv", taskQueue,
//                                         lockTypeEnum);

//   // Run custom tasks
//   ioSystem.customTasks(producerCount, operationCount, readerCount,
//                        consumerCount, operationCount);

//   // Verify data consistency
//   int tasksProduced = ioSystem.getGlobalTaskCounter() - 1;
//   int tasksConsumed = ioSystem.getTasksCompleted();
//   if (tasksProduced != tasksConsumed) {
//     std::cerr << "[customTestFunc] Data inconsistency detected: Produced "
//               << tasksProduced << " tasks but consumed " << tasksConsumed
//               << " tasks." << std::endl;
//   } else {
//     std::cout << "[customTestFunc] All tasks matched: Produced "
//               << tasksProduced << ", Consumed " << tasksConsumed <<
//               std::endl;
//   }

//   std::cout << "[customTestFunc] Test completed for lockType: " << lockType
//             << std::endl;
// }

// void runCustomBenchmark() {
//   vector<string> lockTypes = {"MutexLock", "RWLock"};
//   vector<tuple<int, int, int>> customThreads = {
//       {1, 1, 1}, // 1 producer, 1 consumer, 1 reader
//       {3, 3, 1}, // 4 producers, 4 consumers, 2 readers
//       {4, 2, 4}, // 8 producers, 2 consumers, 4 readers
//       {2, 4, 2}, // 8 producers, 2 consumers, 4 readers

//   };
//   vector<int> customsJobOperationCounts = {10, 100, 1000, 5000};

//   cout << "Running Custom Benchmark...\n" << endl;

//   auto customResults = BenchmarkTool::runCustomBenchmark(
//       "Custom Test", lockTypes, customThreads, customsJobOperationCounts,
//       customTestFunc);

//   BenchmarkTool::exportCustomResultsToCSV("ResultsCustom.csv",
//   customResults);
// }
//--
//--
//--
//--
//--
//--
//--
// main function-----------------------------------------------------
int main() {
  cout << "Starting Benchmarks..." << endl;

  try {
    runThreadBenchmark();
    cout << "-----------------------------------------\n" << endl;

    runIOBenchmark();
    cout << "-----------------------------------------\n" << endl;

    // runCustomBenchmark();
    cout << "-----------------------------------------\n" << endl;

    cout << "Benchmarks completed successfully." << endl;
  } catch (const exception &e) {
    cerr << "Error during benchmark execution: " << e.what() << endl;
    return 1;
  }

  return 0;
}