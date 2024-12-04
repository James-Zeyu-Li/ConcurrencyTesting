// BenchmarkTool.cpp
#include "BenchmarkTool.h"
#include "../cpp/CSVHandler.h"
#include "../cpp/ProducerConsumerConcurrentIO.h"
#include "../cpp/TaskQueue.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

using namespace std;

std::mutex BenchmarkTool::statsMutex;
std::mutex BenchmarkTool::coutMutex;

// Corrected to use make_shared
shared_ptr<TaskQueue> BenchmarkTool::createTaskQueue(const string &lockType) {
  if (lockType == "MutexLock") {
    return make_shared<TaskQueue>(LockType::Mutex, nullptr);
  } else if (lockType == "RWLock") {
    return make_shared<TaskQueue>(LockType::RWLock, nullptr);
  } else {
    {
      std::lock_guard<std::mutex> lock(coutMutex);
      cerr << "Unknown lock type: " << lockType << endl;
    }
    return nullptr;
  }
}

// Run a thread-based benchmark
vector<BenchmarkTool::BenchmarkResult> BenchmarkTool::runThreadBenchmark(
    const string &testName, const vector<string> &lockTypes,
    const vector<pair<int, int>> &threadConfigurations,
    const vector<int> &operationCounts,
    void (*threadTestFunc)(TaskQueue &, int, int, int)) {

  vector<BenchmarkResult> results;

  for (const auto &lockType : lockTypes) {
    for (const auto &[producerCount, consumerCount] : threadConfigurations) {
      for (int operationCount : operationCounts) {
        BenchmarkResult result;
        result.testName = testName;
        result.lockType = lockType;
        result.producerCount = producerCount;
        result.consumerCount = consumerCount;
        result.operationCount = operationCount;

        // Create TaskQueue
        auto taskQueue = createTaskQueue(lockType);
        if (!taskQueue) {
          cerr << "Failed to initialize TaskQueue for lock type: " << lockType
               << endl;
          continue;
        }

        // Record start time
        auto start = chrono::high_resolution_clock::now();

        // Run the test function
        threadTestFunc(*taskQueue, producerCount, consumerCount,
                       operationCount);

        // Collect thread statistics
        collectThreadStatistics(*taskQueue, result);

        // Record end time
        auto end = chrono::high_resolution_clock::now();

        result.totalTime =
            chrono::duration_cast<chrono::microseconds>(end - start).count();
        results.push_back(result);
      }
    }
  }

  return results;
}

// Run an I/O-based benchmark
vector<BenchmarkTool::BenchmarkResult> BenchmarkTool::runIOBenchmark(
    const string &testName, const vector<string> &lockTypes,
    const vector<int> &consumerThreadCounts,
    const vector<int> &readerThreadCounts, const vector<int> &operationCounts,
    void (*ioTestFunc)(CSVHandler &, int, int, int)) {

  vector<BenchmarkResult> results;

  for (const auto &lockType : lockTypes) {
    LockType lockTypeEnum =
        lockType == "MutexLock" ? LockType::Mutex : LockType::RWLock;
    CSVHandler csvHandler("test_io.csv", lockTypeEnum);

    for (int writerCount : consumerThreadCounts) {
      for (int readerCount : readerThreadCounts) {
        for (int operationCount : operationCounts) {
          // 清理统计信息
          BenchmarkResult result;
          result.testName = testName;
          result.lockType = lockType;
          result.consumerCount = writerCount;
          result.readerCount = readerCount;
          result.operationCount = operationCount;

          // 开始测试
          auto start = chrono::high_resolution_clock::now();
          ioTestFunc(csvHandler, writerCount, readerCount, operationCount);
          auto end = chrono::high_resolution_clock::now();

          result.totalTime =
              chrono::duration_cast<chrono::microseconds>(end - start).count();

          collectIOStatistics(csvHandler, result);

          results.push_back(result);

          cout << "Recorded BenchmarkResult for LockType: " << lockType
               << ", Writers: " << writerCount << ", Readers: " << readerCount
               << ", OperationCount: " << operationCount
               << ", TotalTime: " << result.totalTime << " us" << endl;
        }
      }
    }
  }

  return results;
}

// Export thread benchmark results to CSV
void BenchmarkTool::exportThreadResultsToCSV(
    const string &filePath, const vector<BenchmarkResult> &results) {
  ofstream file(filePath);
  if (!file.is_open()) {
    cerr << "Failed to open CSV file for writing: " << filePath << endl;
    return;
  }

  file << "TestName,LockType,ProducerCount,ConsumerCount,OperationCount,"
          "TotalTime(us),"
          "AvgEnqueueTime(us),AvgDequeueTime(us),MaxEnqueueTime(us),"
          "MinEnqueueTime(us),"
          "MaxDequeueTime(us),MinDequeueTime(us),BlockCount,"
          "MaxQueueLength\n";

  for (const auto &result : results) {
    file << result.testName << "," << result.lockType << ","
         << result.producerCount << "," << result.consumerCount << ","
         << result.operationCount << "," << result.totalTime << ","
         << result.avgEnqueueTime << "," << result.avgDequeueTime << ","
         << result.maxEnqueueTime << "," << result.minEnqueueTime << ","
         << result.maxDequeueTime << "," << result.minDequeueTime << ","
         << result.blockCount << "," << result.maxQueueLength << "\n";
  }
  file.close();
}

// Export I/O benchmark results to CSV
void BenchmarkTool::exportIOResultsToCSV(
    const string &filePath, const vector<BenchmarkResult> &results) {
  ofstream file(filePath);
  if (!file.is_open()) {
    cerr << "Failed to open CSV file for writing: " << filePath << endl;
    return;
  }

  // 更新表头
  file << "TestName,LockType,ConsumerCount,ReaderCount,OperationCount,"
          "TotalTime(us),MutexContention,ReadContention,"
          "WriteContention,TotalWriteTime(us),TotalReadTime(us)\n";

  // 更新写入逻辑
  for (const auto &result : results) {
    file << result.testName << "," << result.lockType << ","
         << result.consumerCount << "," << result.readerCount << ","
         << result.operationCount << "," << result.totalTime << ","
         << result.MutexContention << "," << result.RWReadContention << ","
         << result.RWWriteContention << "," << result.totalWriteTime << ","
         << result.totalReadTime << "\n";
  }

  file.close();
}

//--
// ---------------------------------------------------
// Collect statistics for thread benchmark
void BenchmarkTool::collectThreadStatistics(TaskQueue &taskQueue,
                                            BenchmarkResult &result) {
  lock_guard<mutex> lock(statsMutex);

  // Collect enqueue/dequeue timing
  result.avgEnqueueTime = taskQueue.getAverageEnqueueTime();
  result.avgDequeueTime = taskQueue.getAverageDequeueTime();

  result.maxEnqueueTime = taskQueue.getMaxEnqueueTime();
  if (result.maxEnqueueTime == 0) {
    result.maxEnqueueTime = -1; // 表示没有有效数据
  }

  result.minEnqueueTime = taskQueue.getMinEnqueueTime();
  if (result.minEnqueueTime == numeric_limits<long>::max()) {
    result.minEnqueueTime = -1; // 表示没有有效数据
  }

  result.maxDequeueTime = taskQueue.getMaxDequeueTime();
  if (result.maxDequeueTime == 0) {
    result.maxDequeueTime = -1; // 表示没有有效数据
  }

  result.minDequeueTime = taskQueue.getMinDequeueTime();
  if (result.minDequeueTime == numeric_limits<long>::max()) {
    result.minDequeueTime = -1; // 表示没有有效数据
  }

  // Collect lock contention and blocking stats
  result.blockCount = taskQueue.getBlockCount();

  if (taskQueue.getLockType() == LockType::RWLock) {
    RWLock *rwLock = taskQueue.getRWLock();
    if (rwLock) {
      result.readBlockCount = rwLock->getReadContentionByWriteCount();
      result.writeBlockCount = rwLock->getWriteContentionCount();
    }
  }

  // Collect max queue length
  result.maxQueueLength = taskQueue.getMaxQueueLength(); // 新增逻辑
}

// Collect statistics for I/O benchmark
void BenchmarkTool::collectIOStatistics(CSVHandler &csvHandler,
                                        BenchmarkResult &result) {
  LockType lockType = csvHandler.getLockType();

  if (lockType == LockType::Mutex) {
    // 收集 Mutex 锁争用信息
    result.MutexContention = csvHandler.getMutexContention();
  }

  result.totalWriteTime = csvHandler.getTotalWriteTime();
  result.totalReadTime = csvHandler.getTotalReadTime();
}

//--
//--
//--
//--
//--
// Run a custom benchmark
std::vector<BenchmarkTool::BenchmarkResult> BenchmarkTool::runCustomBenchmark(
    const std::string &testName, const std::vector<std::string> &lockTypes,
    const std::vector<std::tuple<int, int, int>> &threadDistributions,
    const std::vector<int> &operationCounts,
    void (*customTestFunc)(const std::string &, std::shared_ptr<TaskQueue>, int,
                           int, int, int)) {

  std::vector<BenchmarkResult> results;

  for (const auto &lockType : lockTypes) {
    for (const auto &[producerCount, consumerCount, readerCount] :
         threadDistributions) {
      for (const auto &operationCount : operationCounts) {
        // Create TaskQueue
        auto taskQueue = createTaskQueue(lockType);
        if (!taskQueue) {
          std::lock_guard<std::mutex> lock(coutMutex);
          std::cerr << "Failed to initialize TaskQueue for lock type: "
                    << lockType << std::endl;
          continue;
        }

        BenchmarkResult result;
        result.testName = testName;
        result.lockType = lockType;
        result.producerCount = producerCount;
        result.consumerCount = consumerCount;
        result.readerCount = readerCount;
        result.operationCount = operationCount;

        // Record start time
        auto start = std::chrono::high_resolution_clock::now();

        // Execute custom test function
        customTestFunc(lockType, taskQueue, producerCount, consumerCount,
                       readerCount, operationCount);

        // Record end time
        auto end = std::chrono::high_resolution_clock::now();

        // Calculate total time
        result.totalTime =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();

        // Initialize I/O system for statistics collection
        LockType lockTypeEnum =
            (lockType == "MutexLock") ? LockType::Mutex : LockType::RWLock;
        ProducerConsumerConcurrentIO ioSystem("custom_test.csv", taskQueue,
                                              lockTypeEnum);

        // Collect statistics
        collectCustomStatistics(ioSystem, result);

        // Verify consistency
        if (result.tasksProduced != result.tasksConsumed) {
          std::lock_guard<std::mutex> lock(coutMutex);
          std::cerr << "[runCustomBenchmark] Data inconsistency detected: "
                    << "Produced " << result.tasksProduced << ", Consumed "
                    << result.tasksConsumed << std::endl;
        }

        // Push result
        results.push_back(result);
      }
    }
  }

  return results;
}

void BenchmarkTool::collectCustomStatistics(
    ProducerConsumerConcurrentIO &ioSystem, BenchmarkResult &result) {
  std::lock_guard<std::mutex> lock(statsMutex);

  result.tasksProduced =
      ProducerConsumerConcurrentIO::getGlobalTaskCounter() - 1;
  result.tasksConsumed = ioSystem.getTasksCompleted();
  result.tasksRead = ioSystem.isReadCompleted() ? result.tasksProduced : 0;
  result.maxQueueLength = ioSystem.getTaskQueue()->getMaxQueueLength();
}

// Export custom benchmark results to CSV
void BenchmarkTool::exportCustomResultsToCSV(
    const string &filePath, const vector<BenchmarkResult> &results) {
  ofstream file(filePath);
  if (!file.is_open()) {
    std::lock_guard<std::mutex> lock(coutMutex);
    cerr << "Failed to open CSV file for writing: " << filePath << endl;
    return;
  }

  // Export header
  file << "TestName,LockType,ProducerCount,ConsumerCount,ReaderCount,"
          "OperationCount,TotalTime(us),ProducerTime(us),ConsumerTime(us),"
          "ReaderTime(us),TasksProduced,TasksConsumed,TasksRead,"
          "MaxQueueLength\n";

  // Export content
  for (const auto &result : results) {
    file << result.testName << "," << result.lockType << ","
         << result.producerCount << "," << result.consumerCount << ","
         << result.readerCount << "," << result.operationCount << ","
         << result.totalTime << "," << result.producerRunningTime << ","
         << result.consumerRunningTime << "," << result.readerRunningTime << ","
         << result.tasksProduced << "," << result.tasksConsumed << ","
         << result.tasksRead << "," << result.maxQueueLength << "\n";
  }

  file.close();
}
