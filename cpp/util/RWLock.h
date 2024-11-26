#ifndef RWLOCK_H
#define RWLOCK_H

#include <pthread.h>

class RWLock {
 private:
  pthread_rwlock_t rwlock;

 public:
  RWLock();
  ~RWLock();

  void readLock();
  void writeLock();
  void readUnlock();
  void writeUnlock();
};

#endif  // RWLOCK_H
