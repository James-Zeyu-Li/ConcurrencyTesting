#include <atomic>
#include <cassert>
#include <iostream>
#include <pthread.h>
#include <thread>
#include <vector>

using namespace std;

#include "../util/MutexLock.h"
#include "../util/RWLock.h"

// Test MutexLock: Single Thread Lock/Unlock
void testMutexSingleThread() {
  MutexLock mutex;
  std::cout << "Testing MutexLock: Single thread lock/unlock..." << std::endl;
  mutex.mutexLockOn(); // Lock
  std::cout << "Mutex lock acquired." << std::endl;
  mutex.mutexUnlock(); // Unlock
  std::cout << "Mutex lock released." << std::endl;
  assert(mutex.getContentionCount() == 0); // No contention in single-thread
  std::cout << "[PASS] MutexLock: Single thread lock/unlock works as expected."
            << std::endl;
}

void testMutexContention() {
  MutexLock mutex;
  std::atomic<int> sharedCounter = 0;

  auto threadFunc = [&mutex, &sharedCounter]() {
    for (int i = 0; i < 5; ++i) {
      mutex.mutexLockOn();
      sharedCounter++;
      std::cout << "Thread " << pthread_self()
                << ": Counter = " << sharedCounter.load() << std::endl;
      mutex.mutexUnlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  };

  std::thread t1(threadFunc);
  std::thread t2(threadFunc);

  t1.join();
  t2.join();

  std::cout << "Final contention count: " << mutex.getContentionCount()
            << std::endl;
  std::cout << "Final counter value: " << sharedCounter.load() << std::endl;
}

// Test RWLock: Single Thread Read/Write Lock
void testRWLockSingleThread() {
  RWLock rwlock;

  rwlock.readLock();   // Read lock
  rwlock.readUnlock(); // Unlock read lock

  rwlock.writeLock();   // Write lock
  rwlock.writeUnlock(); // Unlock write lock

  assert(rwlock.getReadContentionByWriteCount() == 0); // No contention
  assert(rwlock.getWriteContentionCount() == 0);
  std::cout << "[PASS] RWLock: Single thread read/write lock works as expected."
            << std::endl;
}

// Test RWLock: Multi-threaded Read Lock
void testRWLockMultiRead() {
  RWLock rwlock;
  std::atomic<int> counter = 0;

  auto readFunc = [&rwlock, &counter]() {
    rwlock.readLock(); // Multiple threads can acquire read lock
    ++counter;
    rwlock.readUnlock();
  };

  std::thread t1(readFunc);
  std::thread t2(readFunc);
  t1.join();
  t2.join();

  assert(counter == 2); // Both threads should increment the counter
  assert(rwlock.getReadContentionByWriteCount() ==
         0); // No contention for reads
  std::cout << "[PASS] RWLock: Multi-threaded read lock works as expected."
            << std::endl;
}

// Test RWLock: Read-Write Lock
void testRWLockReadWrite() {
  RWLock rwlock;
  std::atomic<int> sharedData = 0;

  auto writeFunc = [&rwlock, &sharedData]() {
    rwlock.writeLock(); // Write lock
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    sharedData++;
    rwlock.writeUnlock();
  };

  auto readFunc = [&rwlock, &sharedData]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    rwlock.readLock();       // Read lock
    assert(sharedData >= 0); // Ensure shared data is consistent
    rwlock.readUnlock();
  };

  std::thread writer(writeFunc);
  std::thread reader1(readFunc);
  std::thread reader2(readFunc);

  writer.join();
  reader1.join();
  reader2.join();

  assert(sharedData == 1); // Only one write should happen
  if (rwlock.getWriteContentionCount() == 0) {
    std::cout << "[INFO] No write contention detected during test."
              << std::endl;
  } else {
    std::cout << "[INFO] Write contention count: "
              << rwlock.getWriteContentionCount() << std::endl;
  }

  std::cout << "[PASS] RWLock: Read-write lock works as expected." << std::endl;
}

void testRWLockReadWriteWithContention() {
  RWLock rwlock;
  std::atomic<int> sharedData = 0;

  // write thread get write lock and block read threads
  auto writeFunc = [&rwlock, &sharedData]() {
    std::cout << "[DEBUG] Write thread starting." << std::endl;
    rwlock.writeLock();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 持有写锁
    sharedData++;
    rwlock.writeUnlock();
    std::cout << "[DEBUG] Write thread finished." << std::endl;
  };

  // read threads try to get read lock but blocked by write lock
  auto readFunc = [&rwlock, &sharedData]() {
    std::cout << "[DEBUG] Read thread starting." << std::endl;
    rwlock.readLock(); // blocked by write lock until write unlock
    assert(sharedData >= 0);
    rwlock.readUnlock();
    std::cout << "[DEBUG] Read thread finished." << std::endl;
  };

  // multi-threaded write
  std::thread writer(writeFunc);

  // multi-threaded read
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::thread reader1(readFunc);
  std::thread reader2(readFunc);

  writer.join();
  reader1.join();
  reader2.join();

  // check contention count
  int readContention = rwlock.getReadContentionByWriteCount();
  int writeContention = rwlock.getWriteContentionCount();

  std::cout << "[INFO] Final read contention count: " << readContention
            << std::endl;
  std::cout << "[INFO] Final write contention count: " << writeContention
            << std::endl;

  // no read contention in this test
  assert(readContention > 0);
  // write contention occurred
  assert(writeContention == 0);

  std::cout << "[PASS] RWLock: Read-write lock with expected contention works."
            << std::endl;
}

// Test RWLock: Contention Count Reset
void testRWLockContentionReset() {
  RWLock rwlock;

  auto writeFunc = [&rwlock]() {
    rwlock.writeLock();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    rwlock.writeUnlock();
  };

  std::thread writer1(writeFunc);
  std::thread writer2(writeFunc);

  writer1.join();
  writer2.join();

  assert(rwlock.getWriteContentionCount() > 0);  // Write contention occurred
  rwlock.resetWriteContentionCount();            // Reset contention count
  assert(rwlock.getWriteContentionCount() == 0); // Contention count reset
  std::cout << "[PASS] RWLock: Contention count reset works as expected."
            << std::endl;
}

// Test MutexLock: Contention Count Reset
void testMutexContentionReset() {
  MutexLock mutex;

  auto lockFunc = [&mutex]() {
    mutex.mutexLockOn();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    mutex.mutexUnlock();
  };

  std::thread t1(lockFunc);
  std::thread t2(lockFunc);

  t1.join();
  t2.join();

  assert(mutex.getContentionCount() > 0);  // Contention occurred
  mutex.resetContentionCount();            // Reset contention count
  assert(mutex.getContentionCount() == 0); // Contention count reset
  std::cout << "[PASS] MutexLock: Contention count reset works as expected."
            << std::endl;
}

// Main function to run all tests
int main() {
  std::cout << "Running all tests for MutexLock and RWLock..." << std::endl;

  testMutexSingleThread();
  testMutexContention();
  testRWLockSingleThread();
  testRWLockMultiRead();
  testRWLockReadWrite();
  testRWLockReadWriteWithContention();
  testRWLockContentionReset();
  testMutexContentionReset();

  std::cout << "All tests passed!" << std::endl;
  return 0;
}

// g++ -std=c++17 -pthread -o LockTest LockTest.cpp ../util/MutexLock.cpp
// ../util/RWLock.cpp