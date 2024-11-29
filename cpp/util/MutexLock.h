#ifndef MUTEXLOCK_H
#define MUTEXLOCK_H

#include <pthread.h>

class MutexLock {
 private:
  pthread_mutex_t mutex;

 public:
  MutexLock();
  void mutexLockOn();
  void mutexUnlock();
  ~MutexLock();

  void waitOnCondition(pthread_cond_t *cond);
};

#endif  // MUTEXLOCK_H
