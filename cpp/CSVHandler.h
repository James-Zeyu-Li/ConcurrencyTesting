#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include "util/LockType.h"
#include "util/MutexLock.h"
#include "util/RWLock.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum class LockOperation { Read, Write };

class CSVHandler {
private:
  std::string filePath;    // File path for CSV
  LockType lockType;       // Lock type (Mutex or RWLock)
  MutexLock fileMutex;     // Mutex lock for file operations
  RWLock fileRWLock;       // Read-write lock for file operations
  std::fstream fileStream; // File stream for reading and writing

  // Benchmark statistics
  std::atomic<long> totalWriteTime{
      0}; // Total time spent on write operations (us)
  std::atomic<long> totalReadTime{
      0}; // Total time spent on read operations (us)
  std::atomic<long> maxWriteTime{
      0}; // Maximum time spent on a single write operation (us)
  std::atomic<long> minWriteTime{
      LONG_MAX}; // Minimum time spent on a single write operation (us)
  std::atomic<long> maxReadTime{
      0}; // Maximum time spent on a single read operation (us)
  std::atomic<long> minReadTime{
      LONG_MAX}; // Minimum time spent on a single read operation (us)
  std::atomic<int> writeCount{0}; // Number of write operations
  std::atomic<int> readCount{0};  // Number of read operations

  // Lock and unlock helpers
  void lock(LockType lockType, LockOperation operation);
  void unlock(LockType lockType, LockOperation operation);

public:
  // Constructor and destructor
  CSVHandler(const std::string &path, LockType lockType = LockType::Mutex);
  ~CSVHandler();

  // Getters for file path and lock type
  RWLock *getRWLock();
  MutexLock *getMutexLock();

  // Core functionalities for CSV handling
  void writeRow(const std::vector<std::string> &row); // Write a row to the CSV
  std::vector<std::vector<std::string>> readAll(); // Read all rows from the CSV
  void clear();       // Clear the content of the CSV file
  void resetStream(); // Reset the file stream pointer
  void closeStream(); // Close the file stream
  //----------------------------------------------

  // Getters for benchmark statistics
  long getTotalWriteTime() const; // Get total write time
  long getTotalReadTime() const;  // Get total read time
  long getMaxWriteTime() const;   // Get maximum write time
  long getMinWriteTime() const;   // Get minimum write time
  long getMaxReadTime() const;    // Get maximum read time
  long getMinReadTime() const;    // Get minimum read time
  int getWriteCount() const;      // Get number of write operations
  int getReadCount() const;       // Get number of read operations

  // Lock contention statistics (if supported by MutexLock and RWLock)
  int getMutexContention() const;   // Get Mutex contention count
  int getRWReadContention() const;  // Get RWLock read contention count
  int getRWWriteContention() const; // Get RWLock write contention count

  LockType getLockType() const; // Get the type of lock
};

#endif // CSVHANDLER_H
