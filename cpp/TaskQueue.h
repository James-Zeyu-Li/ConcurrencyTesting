#ifndef TASKQUEUE_H
#define TASKQUEUE_H
#include "util/LockType.h"
#include "util/MutexLock.h"
#include "util/RWLock.h"

#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <string>

struct Task {
  int id;
  std::string name;
  bool isCompleted;
};

class TaskQueue {
private:
  std::queue<Task> tasksQueue;
  LockType lockType; // type of lock, mutex or rwlock
  MutexLock *mutexLock;
  RWLock *rwLock;
  bool isExternalLock; // check if the lock is external

  pthread_cond_t cond;        // provide wait and signal functionality
  pthread_mutex_t queueMutex; // mutex for condition variable, for thread safety

  // Benchmark data
  std::atomic<long> totalEnqueueTime{0}; // Total enqueue operation time
  std::atomic<long> totalDequeueTime{0}; // Total dequeue operation time
  std::atomic<int> enqueueCount{0};      // Total enqueue operations
  std::atomic<int> dequeueCount{0};      // Total dequeue operations

  std::atomic<long> maxEnqueueTime{0};        // Max enqueue time
  std::atomic<long> minEnqueueTime{LONG_MAX}; // Min enqueue time
  std::atomic<long> maxDequeueTime{0};        // Max dequeue time
  std::atomic<long> minDequeueTime{LONG_MAX}; // Min dequeue time

  std::atomic<int> blockCount{0}; // Number of times the queue was blocked
  std::atomic<int> maxQueueLength{0};

public:
  TaskQueue(LockType type, void *lock = nullptr);
  ~TaskQueue(); // destructor

  void lock();   // lock the queue, based on the lock type
  void unlock(); // unlock the queue, based on the lock type

  void enqueue(const Task &t);
  bool dequeue(Task &t);
  void dequeueAll();

  bool isEmpty();
  int queueSize(); // to get how many tasks are in the queue

  // Benchmark Tools
  long getTotalEnqueueTime() const;
  long getTotalDequeueTime() const;
  double getAverageEnqueueTime() const;
  double getAverageDequeueTime() const;
  long getMaxEnqueueTime() const;
  long getMinEnqueueTime() const;
  long getMaxDequeueTime() const;
  long getMinDequeueTime() const;
  int getBlockCount() const;

  // Lock management
  MutexLock *getMutexLock() const;
  RWLock *getRWLock() const;

  LockType getLockType() const { return lockType; }
  int getMaxQueueLength() const;
};

#endif // TASKQUEUE_H