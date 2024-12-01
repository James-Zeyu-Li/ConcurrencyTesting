#ifndef TASKQUEUE_H
#define TASKQUEUE_H
#include "util/LockType.h"
#include "util/MutexLock.h"
#include "util/RWLock.h"

#include <iostream>
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

  pthread_cond_t cond;       // provide wait and signal functionality
  pthread_mutex_t queueMutex; // mutex for condition variable, for thread safety

  void lock();   // lock the queue, based on the lock type
  void unlock(); // unlock the queue, based on the lock type

  bool isExternalLock; // check if the lock is external

public:
  TaskQueue(LockType type, void *lock = nullptr);
  ~TaskQueue(); // destructor

  void enqueue(const Task &t);

  bool dequeue(Task &t);

  void dequeueAll();

  bool isEmpty();

  int queueSize(); // to get how many tasks are in the queue
};

#endif // TASKQUEUE_H