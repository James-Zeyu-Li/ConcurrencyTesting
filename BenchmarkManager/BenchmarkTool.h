// BenchmarkTool.h
#ifndef BENCHMARK_TOOL_H
#define BENCHMARK_TOOL_H

#include "../cpp/CSVHandler.h"
#include "../cpp/ProducerConsumerConcurrentIO.h"
#include "../cpp/TaskQueue.h"
#include <functional>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class BenchmarkTool {
public:
  struct BenchmarkResult {
    std::string testName;
    std::string lockType;
    int producerCount = 0; // Producer thread count
    int consumerCount = 0; // Consumer thread count
    int readerCount = 0;   // Reader thread count
    int operationCount;    // Total operation count
    long totalTime;        // Total execution time

    long avgEnqueueTime = 0; // Average enqueue time
    long avgDequeueTime = 0; // Average dequeue time
    long maxEnqueueTime = 0; // Max enqueue time
    long minEnqueueTime = std::numeric_limits<long>::max(); // Min enqueue time
    long maxDequeueTime = 0;                                // Max dequeue time
    long minDequeueTime = std::numeric_limits<long>::max(); // Min dequeue time
    long lockWaitTime = 0;    // Total time spent waiting for lock
    int blockCount = 0;       // Number of times the queue was blocked
    int readBlockCount = 0;   // Number of times the queue was blocked for read
    int writeBlockCount = 0;  // Number of times the queue was blocked for write
    int MutexContention = 0;  // Mutex contention count
    int RWReadContention = 0; // RWLock read contention count
    int RWWriteContention = 0; // RWLock write contention count

    long producerRunningTime = 0;
    long consumerRunningTime = 0;
    long readerRunningTime = 0;
    int tasksProduced = 0;
    int tasksConsumed = 0;
    int tasksRead = 0;
    long totalIOWriteTime = 0;
    long totalWriteTime = 0;
    long totalReadTime = 0;
    int maxQueueLength = 0;
  };

  static std::mutex statsMutex;
  static std::mutex coutMutex;

  // Corrected to return shared_ptr<TaskQueue>
  static std::shared_ptr<TaskQueue>
  createTaskQueue(const std::string &lockType);

  // Run a thread-based benchmark
  static std::vector<BenchmarkResult> runThreadBenchmark(
      const std::string &testName, const std::vector<std::string> &lockTypes,
      const std::vector<std::pair<int, int>> &threadConfigurations,
      const std::vector<int> &operationCounts,
      void (*threadTestFunc)(TaskQueue &, int, int, int));

  // Run an I/O-based benchmark
  static std::vector<BenchmarkResult>
  runIOBenchmark(const std::string &testName,
                 const std::vector<std::string> &lockTypes,
                 const std::vector<int> &consumerThreadCounts,
                 const std::vector<int> &readerThreadCounts,
                 const std::vector<int> &operationCounts,
                 void (*ioTestFunc)(CSVHandler &, int, int, int));

  // Run a custom benchmark with corrected signature
  static std::vector<BenchmarkResult> runCustomBenchmark(
      const std::string &testName, const std::vector<std::string> &lockTypes,
      const std::vector<std::tuple<int, int, int>> &threadDistributions,
      const std::vector<int> &operationCounts,
      void (*customTestFunc)(const std::string &, std::shared_ptr<TaskQueue>,
                             int, int, int, int));

  // Export results to CSV
  static void
  exportThreadResultsToCSV(const std::string &filePath,
                           const std::vector<BenchmarkResult> &results);
  static void exportIOResultsToCSV(const std::string &filePath,
                                   const std::vector<BenchmarkResult> &results);
  static void
  exportCustomResultsToCSV(const std::string &filePath,
                           const std::vector<BenchmarkResult> &results);

  // Collect statistics
  static void collectThreadStatistics(TaskQueue &taskQueue,
                                      BenchmarkResult &result);
  static void collectIOStatistics(CSVHandler &csvHandler,
                                  BenchmarkResult &result);
  static void collectCustomStatistics(ProducerConsumerConcurrentIO &ioSystem,
                                      BenchmarkResult &result);
};

#endif // BENCHMARK_TOOL_H