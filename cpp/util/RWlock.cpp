#include "RWLock.h"
#include <stdexcept> // For exception handling

using namespace std;

RWLock::RWLock() {
  if (pthread_rwlock_init(&rwlock, nullptr) != 0) {
    throw runtime_error("Failed to initialize read-write lock");
  }
}

RWLock::~RWLock() { pthread_rwlock_destroy(&rwlock); }

void RWLock::readLock() {
  if (pthread_rwlock_tryrdlock(&rwlock) != 0) {
    readContentionCount++; // if readlock fail, increase count
  }
  pthread_rwlock_rdlock(&rwlock);
}

void RWLock::writeLock() {
  if (pthread_rwlock_trywrlock(&rwlock) != 0) {
    writeContentionCount++; // if writelock fail, increase count
  }
  pthread_rwlock_wrlock(&rwlock);
}

void RWLock::readUnlock() { pthread_rwlock_unlock(&rwlock); }

void RWLock::writeUnlock() { pthread_rwlock_unlock(&rwlock); }

int RWLock::getReadContentionCount() const { return readContentionCount; }

int RWLock::getWriteContentionCount() const { return writeContentionCount; }