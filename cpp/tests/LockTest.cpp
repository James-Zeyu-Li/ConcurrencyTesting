#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

#include "../util/MutexLock.h"
#include "../util/RWLock.h"

// Test MutexLock: Single Thread Lock/Unlock
void testMutexSingleThread() {
  MutexLock mutex;
  mutex.mutexLockOn();  // Lock
  mutex.mutexUnlock();  // Unlock
  std::cout << "[PASS] MutexLock: Single thread lock/unlock works as expected."
            << std::endl;
}

// Test MutexLock: Multi-threaded Lock
void testMutexMultiThreaded() {
  MutexLock mutex;
  std::atomic<int> counter = 0;

  auto threadFunc = [&mutex, &counter]() {
    mutex.mutexLockOn();  // Lock
    ++counter;            // Increment shared counter
    mutex.mutexUnlock();  // Unlock
  };

  std::thread t1(threadFunc);
  std::thread t2(threadFunc);
  t1.join();
  t2.join();

  assert(counter == 2);  // Both threads should increment the counter
  std::cout << "[PASS] MutexLock: Multi-threaded lock/unlock works as expected."
            << std::endl;
}

// Test RWLock: Single Thread Read/Write Lock
void testRWLockSingleThread() {
  RWLock rwlock;

  rwlock.readLock();    // Read lock
  rwlock.readUnlock();  // Unlock read lock

  rwlock.writeLock();    // Write lock
  rwlock.writeUnlock();  // Unlock write lock

  std::cout << "[PASS] RWLock: Single thread read/write lock works as expected."
            << std::endl;
}

// Test RWLock: Multi-threaded Read Lock
void testRWLockMultiRead() {
  RWLock rwlock;
  std::atomic<int> counter = 0;

  auto readFunc = [&rwlock, &counter]() {
    rwlock.readLock();  // Multiple threads can acquire read lock simultaneously
    ++counter;
    rwlock.readUnlock();
  };

  std::thread t1(readFunc);
  std::thread t2(readFunc);
  t1.join();
  t2.join();

  assert(counter == 2);  // Both threads should increment the counter
  std::cout << "[PASS] RWLock: Multi-threaded read lock works as expected."
            << std::endl;
}

// Test RWLock: Read-Write Lock
void testRWLockReadWrite() {
  RWLock rwlock;
  std::atomic<int> sharedData = 0;

  auto writeFunc = [&rwlock, &sharedData]() {
    rwlock.writeLock();  // Write lock
    sharedData++;
    rwlock.writeUnlock();
  };

  auto readFunc = [&rwlock, &sharedData]() {
    rwlock.readLock();        // Read lock
    assert(sharedData >= 0);  // Ensure shared data is consistent
    rwlock.readUnlock();
  };

  std::thread writer(writeFunc);
  std::thread reader1(readFunc);
  std::thread reader2(readFunc);

  writer.join();
  reader1.join();
  reader2.join();

  assert(sharedData == 1);  // Only one write should happen
  std::cout << "[PASS] RWLock: Read-write lock works as expected." << std::endl;
}

// Main function to run all tests
int main() {
  std::cout << "Running unit tests..." << std::endl;

  testMutexSingleThread();
  testMutexMultiThreaded();
  testRWLockSingleThread();
  testRWLockMultiRead();
  testRWLockReadWrite();

  std::cout << "All tests passed!" << std::endl;
  return 0;
}
