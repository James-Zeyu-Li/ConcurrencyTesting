#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H

#include <atomic>
#include <pthread.h>

class MutexLock {
private:
  pthread_mutex_t mutex;
  std::atomic<int> mutexContentionCount; // record mutex lock contention

public:
  MutexLock();
  ~MutexLock();

  // can't allow copy constructor and assignment operator
  MutexLock(const MutexLock &) = delete;
  MutexLock &operator=(const MutexLock &) = delete;

  void mutexLockOn();
  void mutexUnlock();

  void waitOnCondition(pthread_cond_t *cond); // wait on condition variable

  int getContentionCount() const; // get the contention count
  int resetContentionCount();     // reset the contention count
};

#endif // MUTEXLOCK_H
