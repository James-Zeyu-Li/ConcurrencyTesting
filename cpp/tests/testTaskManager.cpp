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

// Test case: Basic functionality with MutexLock
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

// Test case: Multiple producers and consumers
void testMultipleProducersConsumers() {
  printSection("Test: Multiple Producers and Consumers");

  TaskQueue queue(LockType::RWLock); // Using RWLock for this test
  TaskThreadManager manager("test_rwlock.csv", queue);

  // Start 5 producer threads, each producing 10 tasks
  for (int i = 0; i < 5; ++i) {
    manager.startProducerThread(10);
  }

  // Start 3 consumer threads
  for (int i = 0; i < 3; ++i) {
    manager.startConsumerThread();
  }

  // Start 2 reader threads
  for (int i = 0; i < 2; ++i) {
    manager.startReaderThread();
  }

  // Wait for threads to process tasks
  this_thread::sleep_for(chrono::seconds(5));

  // Stop threads
  manager.stopConsumerThread(3); // Stop all consumer threads
  manager.stopReaderThread();

  // Verify CSV content
  auto rows = manager.getCSVContent();
  assert(rows.size() == 50); // No header, tasks only
  cout << "Verified CSV content for RWLock successfully." << endl;
}

// Test case: Edge cases
void testEdgeCases() {
  printSection("Test: Edge Cases");

  TaskQueue queue(LockType::Mutex);
  TaskThreadManager manager("test_edge.csv", queue);

  // Case 1: Stop producer early
  manager.startProducerThread(5); // Start producing 5 tasks
  this_thread::sleep_for(
      chrono::milliseconds(200)); // Let it produce some tasks
  manager.stopProducerThread();   // Stop the producer

  // Case 2: Stop consumer early
  manager.startConsumerThread();
  this_thread::sleep_for(
      chrono::milliseconds(200)); // Let it consume some tasks
  manager.stopConsumerThread(1);  // Stop the consumer

  // Case 3: Empty queue
  assert(queue.isEmpty()); // Ensure queue is empty
  cout << "Verified empty queue after stopping producer and consumer." << endl;

  // Case 4: Reader on non-existent file
  remove("test_edge.csv"); // Ensure file does not exist
  try {
    auto rows = manager.getCSVContent();
    cout << "Unexpected success in reading non-existent file." << endl;
  } catch (const runtime_error &e) {
    cout << "Caught expected runtime_error: " << e.what() << endl;
  } catch (...) {
    cerr << "Caught unexpected exception type." << endl;
  }
}

// Test case: High load and stress test
void testStress() {
  printSection("Test: High Load and Stress Test");

  TaskQueue queue(LockType::RWLock); // Using RWLock for stress test
  TaskThreadManager manager("test_stress.csv", queue);

  // Start 10 producer threads, each producing 50 tasks
  for (int i = 0; i < 10; ++i) {
    manager.startProducerThread(50);
  }

  // Start 10 consumer threads
  for (int i = 0; i < 10; ++i) {
    manager.startConsumerThread();
  }

  // Start 5 reader threads
  for (int i = 0; i < 5; ++i) {
    manager.startReaderThread();
  }

  // Wait for threads to process tasks
  this_thread::sleep_for(chrono::seconds(10));

  // Stop threads
  manager.stopConsumerThread(10); // Stop all consumer threads
  manager.stopReaderThread();

  // Verify CSV content
  auto rows = manager.getCSVContent();
  assert(rows.size() == 500); // No header, only tasks
  cout << "Verified CSV content under stress successfully." << endl;
}

// Test all cases
void comprehensiveTest() {
  printSection("Comprehensive Test");

  testBasicFunctionalityWithMutex();
  testMultipleProducersConsumers();
  testEdgeCases();
  testStress();

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