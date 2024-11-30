#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H

#include <pthread.h>

class MutexLock {
private:
  pthread_mutex_t mutex;
  int mutexContentionCount; // record mutex lock contention count

public:
  MutexLock();
  ~MutexLock();

  void mutexLockOn();
  void mutexUnlock();

  void waitOnCondition(pthread_cond_t *cond); // wait on condition variable

  int getContentionCount() const; // get the contention count
};

#endif // MUTEXLOCK_H
