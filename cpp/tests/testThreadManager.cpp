#include "../util/ThreadManager.h"
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

using namespace std;

// Test function for threads
void *simpleRoutine(void *arg) {
  int *data = static_cast<int *>(arg);
  cout << "Thread started with data: " << *data << endl;
  this_thread::sleep_for(chrono::seconds(1)); // Simulate work
  cout << "Thread completed with data: " << *data << endl;
  return nullptr;
}

// Helper function for edge cases
void *edgeRoutine(void *arg) {
  int *data = static_cast<int *>(arg);
  if (*data < 0) {
    throw runtime_error("Negative data encountered!");
  }
  cout << "Edge routine data: " << *data << endl;
  return nullptr;
}

// Test: Basic functionality
void testBasicFunctionality() {
  cout << "\n===== Test: Basic Functionality =====\n" << endl;

  ThreadManager threadManager;
  int data = 42;

  // Create a single thread
  threadManager.createThread(&simpleRoutine, &data, nullptr);
  threadManager.printThreadStatus();

  // Wait for the thread to finish
  this_thread::sleep_for(chrono::seconds(2)); // Allow thread to complete

  cout << "Basic functionality test completed." << endl;
}

// Test: Multiple threads
void testMultipleThreads() {
  cout << "\n===== Test: Multiple Threads =====\n" << endl;

  ThreadManager threadManager;
  vector<int> threadData = {1, 2, 3, 4, 5};

  // Create multiple threads
  for (int &data : threadData) {
    threadManager.createThread(&simpleRoutine, &data, nullptr);
  }
  threadManager.printThreadStatus();

  // Allow threads to finish
  this_thread::sleep_for(chrono::seconds(2));

  cout << "Multiple threads test completed." << endl;
}

// Test: Edge cases
void testEdgeCases() {
  cout << "\n===== Test: Edge Cases =====\n" << endl;

  ThreadManager threadManager;
  int validData = 10;
  int invalidData = -1;

  // Create a thread with valid data
  threadManager.createThread(&edgeRoutine, &validData, nullptr);

  try {
    // Create a thread with invalid data
    threadManager.createThread(&edgeRoutine, &invalidData, nullptr);
  } catch (const exception &e) {
    cout << "Caught exception: " << e.what() << endl;
  }

  threadManager.printThreadStatus();

  // Allow threads to finish
  this_thread::sleep_for(chrono::seconds(2));

  cout << "Edge cases test completed." << endl;
}

// Test: Thread joining
void testThreadJoining() {
  cout << "\n===== Test: Thread Joining =====\n" << endl;

  ThreadManager threadManager;
  int data = 99;

  pthread_t thread;
  threadManager.createThread(&simpleRoutine, &data, &thread);

  // Join the thread
  threadManager.joinThread(thread);

  assert(threadManager.getThreadStatus(thread) == "Finished");
  cout << "Thread successfully joined." << endl;

  threadManager.printThreadStatus();
}

// Test: Status management
void testStatusManagement() {
  cout << "\n===== Test: Status Management =====\n" << endl;

  ThreadManager threadManager;
  int data1 = 1, data2 = 2;

  pthread_t thread1, thread2;
  threadManager.createThread(&simpleRoutine, &data1, &thread1);
  threadManager.createThread(&simpleRoutine, &data2, &thread2);

  threadManager.joinThread(thread1);
  threadManager.joinThread(thread2);

  assert(threadManager.getThreadStatus(thread1) == "Finished");
  assert(threadManager.getThreadStatus(thread2) == "Finished");

  threadManager.printThreadStatus();

  cout << "Status management test completed successfully." << endl;
}

// Test: Stress testing
void testStress() {
  cout << "\n===== Test: Stress Test =====\n" << endl;

  ThreadManager threadManager;
  vector<int> threadData(50);

  // Fill data with unique values
  for (int i = 0; i < 50; ++i) {
    threadData[i] = i + 1;
  }

  // Create a large number of threads
  for (int &data : threadData) {
    threadManager.createThread(&simpleRoutine, &data, nullptr);
  }

  threadManager.printThreadStatus();

  // Allow threads to finish
  this_thread::sleep_for(chrono::seconds(5));

  cout << "Stress test completed." << endl;
}

// Main function to run all tests
int main() {
  try {
    testBasicFunctionality();
    testMultipleThreads();
    testEdgeCases();
    testThreadJoining();
    testStatusManagement();
    testStress();
    cout << "\nAll tests completed successfully!" << endl;
  } catch (const exception &e) {
    cerr << "Test failed: " << e.what() << endl;
  }

  return 0;
}

// g++ -std=c++17 -Wall -Wextra -o testThreadManager ../util/ThreadManager.cpp
// testThreadManager.cpp -pthread
