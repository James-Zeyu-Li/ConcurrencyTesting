#include "../cpp/CSVHandler.h"
#include "../cpp/ProducerConsumerConcurrentIO.h"
#include "../cpp/TaskQueue.h"
#include "BenchmarkTool.h"
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using namespace std;

int main() {
  BenchmarkTool benchmarkTool;

  vector<string> lockTypes = {"MutexLock", "RWLock"};
  vector<int> writerCounts = {1, 2};
  vector<int> readerCounts = {1, 2};
  vector<int> operationCounts = {10, 100};

  auto ioTestFunc = [](CSVHandler &csvHandler, int writerCount,
                       int readerCount, int operationCount) {
    vector<thread> threads;

    for (int i = 0; i < writerCount; ++i) {
      threads.emplace_back([&csvHandler, operationCount, i]() {
        for (int j = 0; j < operationCount; ++j) {
          csvHandler.writeRow({"Writer_" + to_string(i),
                               "Row_" + to_string(j)});
        }
      });
    }

    for (int i = 0; i < readerCount; ++i) {
      threads.emplace_back([&csvHandler, i]() {
        for (int k = 0; k < 5; ++k) {
          auto rows = csvHandler.readAll();
          cout << "Reader_" << i << " read " << rows.size() << " rows." << endl;
        }
      });
    }

    for (auto &t : threads) {
      t.join();
    }
  };

  vector<BenchmarkTool::BenchmarkResult> results =
      benchmarkTool.runIOBenchmark("IO Test", lockTypes, writerCounts,
                                   readerCounts, operationCounts, ioTestFunc);

  benchmarkTool.exportIOResultsToCSV("benchmark_results.csv", results);

  return 0;
}


// g++ -std=c++17 -o test test.cpp BenchmarkTool.cpp ../cpp/CSVHandler.cpp
// ../cpp/util/MutexLock.cpp ../cpp/util/RWLock.cpp -pthread

// clang++ -std=c++17 -o test test.cpp BenchmarkTool.cpp ../cpp/TaskQueue.cpp ../cpp/ProducerConsumerConcurrentIO.cpp ../cpp/CSVHandler.cpp ../cpp/util/MutexLock.cpp ../cpp/util/RWLock.cpp ../cpp/util/ThreadManager.cpp -lpthread
