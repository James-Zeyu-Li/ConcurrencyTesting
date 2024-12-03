#ifndef RWLOCK_H
#define RWLOCK_H

#include <atomic>
#include <pthread.h>

class RWLock {
private:
  pthread_rwlock_t rwlock;
  std::atomic<int> readContentionByWriteCount{0};
  std::atomic<int> writeContentionCount{0};

public:
  RWLock();
  ~RWLock();

  RWLock(const RWLock &) = delete;
  RWLock &operator=(const RWLock &) = delete;

  void readLock();
  void writeLock();
  void readUnlock();
  void writeUnlock();

  int getReadContentionByWriteCount() const;
  int getWriteContentionCount() const;
  int resetReadContentionByWriteCount();
  int resetWriteContentionCount();
};

#endif // RWLOCK_H
