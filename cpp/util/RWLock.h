#ifndef RWLOCK_H
#define RWLOCK_H

#include <pthread.h>

class RWLock {
private:
  pthread_rwlock_t rwlock;
  int readContentionCount;  // record read lock contention count
  int writeContentionCount; // record write lock contention count

public:
  RWLock();
  ~RWLock();

  void readLock();
  void writeLock();
  void readUnlock();
  void writeUnlock();

  int getReadContentionCount() const;
  int getWriteContentionCount() const;
};

#endif // RWLOCK_H
