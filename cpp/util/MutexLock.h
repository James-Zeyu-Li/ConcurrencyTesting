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
};

#endif  // MUTEXLOCK_H
