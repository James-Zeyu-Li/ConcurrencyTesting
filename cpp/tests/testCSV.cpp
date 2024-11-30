#include "../CSVHandler.h"
#include <cassert>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// Forward declaration of the function
void testCSVHandlerWithLockTypeOnSameFile(LockType lockType,
                                          const std::string &lockName);

// Print separator for better test output
void printSeparator(const std::string &testName) {
  std::cout << "\n===== " << testName << " =====\n" << std::endl;
}

// Test CSVHandler with a specific LockType on the same file
void testCSVHandlerWithLockTypeOnSameFile(LockType lockType,
                                          const std::string &lockName) {
  printSeparator("Test CSVHandler with LockType: " + lockName);

  const std::string testFilePath = "test_shared.csv";
  CSVHandler csvHandler(testFilePath, lockType);

  // Step 1: Clear the file to start with a clean slate
  std::cout << "[Test-" << lockName << "] Clearing file..." << std::endl;
  csvHandler.clear();

  // Step 2: Write data to the file
  std::cout << "[Test-" << lockName << "] Writing rows to CSV..." << std::endl;
  csvHandler.writeRow({"ID", "Name", "Status"});
  csvHandler.writeRow({"1", "Task1", "Incomplete"});
  csvHandler.writeRow({"2", "Task2", "Complete"});
  std::cout << "[Test-" << lockName << "] Rows written successfully."
            << std::endl;

  // Step 3: Read data from the file
  std::cout << "[Test-" << lockName << "] Reading rows from CSV..."
            << std::endl;
  auto rows = csvHandler.readAll();
  assert(rows.size() == 3);
  assert(rows[0] == std::vector<std::string>({"ID", "Name", "Status"}));
  assert(rows[1] == std::vector<std::string>({"1", "Task1", "Incomplete"}));
  assert(rows[2] == std::vector<std::string>({"2", "Task2", "Complete"}));
  std::cout << "[Test-" << lockName << "] Rows read and verified successfully."
            << std::endl;

  // Step 4: Simulate concurrent operations
  std::cout << "[Test-" << lockName << "] Simulating concurrent writes..."
            << std::endl;
  std::thread writer1([&csvHandler]() {
    for (int i = 3; i <= 5; ++i) {
      csvHandler.writeRow(
          {std::to_string(i), "Task" + std::to_string(i), "Complete"});
    }
  });
  std::thread writer2([&csvHandler]() {
    for (int i = 6; i <= 8; ++i) {
      csvHandler.writeRow(
          {std::to_string(i), "Task" + std::to_string(i), "Incomplete"});
    }
  });

  writer1.join();
  writer2.join();
  std::cout << "[Test-" << lockName
            << "] Concurrent writes completed. Verifying results..."
            << std::endl;

  rows = csvHandler.readAll();
  assert(rows.size() >= 8); // Ensure at least 8 rows are written
  std::cout << "[Test-" << lockName << "] Concurrent writes verified."
            << std::endl;

  // Step 5: Clear the file again
  std::cout << "[Test-" << lockName << "] Clearing the file..." << std::endl;
  csvHandler.clear();
  rows = csvHandler.readAll();
  assert(rows.empty());
  std::cout << "[Test-" << lockName
            << "] File cleared and verified successfully." << std::endl;
}

// Comprehensive test for CSVHandler with edge cases
void testCSVHandler() {
  const std::string testFilePath = "test_shared.csv";

  // Ensure a clean file to start
  std::ofstream clearFile(testFilePath, std::ios::trunc);
  clearFile.close();

  // Test with Mutex lock
  testCSVHandlerWithLockTypeOnSameFile(LockType::Mutex, "Mutex");

  // Test with RWLock (reader-writer lock)
  testCSVHandlerWithLockTypeOnSameFile(LockType::RWLock, "RWLock");

  // Test with NoLock (no locking mechanism)
  testCSVHandlerWithLockTypeOnSameFile(LockType::NoLock, "NoLock");
}

int main() {
  try {
    testCSVHandler();
    std::cout << "\n[Main] All CSVHandler tests passed!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "[Main] Test failed: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

// g++ -std=c++17 -Wall -Wextra -o testCSV \
//     ../CSVHandler.cpp \
//     ../util/LockImpl/MutexLock.cpp \
//     ../util/LockImpl/RWLock.cpp \
//     testCSV.cpp