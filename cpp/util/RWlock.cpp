#include "RWLock.h"
#include <atomic>
#include <iostream>  // For debugging
#include <stdexcept> // For exception handling

using namespace std;

RWLock::RWLock() : readContentionByWriteCount(0), writeContentionCount(0) {
  if (pthread_rwlock_init(&rwlock, nullptr) != 0) {
    throw runtime_error("Failed to initialize read-write lock");
  }
}

RWLock::~RWLock() { pthread_rwlock_destroy(&rwlock); }

void RWLock::readLock() {
  if (pthread_rwlock_tryrdlock(&rwlock) != 0) {
    readContentionByWriteCount++;
    pthread_rwlock_rdlock(&rwlock); // Block until read lock is acquired
  }
}

void RWLock::writeLock() {
  if (pthread_rwlock_trywrlock(&rwlock) != 0) {
    writeContentionCount++;
    pthread_rwlock_wrlock(&rwlock); // Block until write lock is acquired
  }
}

void RWLock::readUnlock() { pthread_rwlock_unlock(&rwlock); }

void RWLock::writeUnlock() { pthread_rwlock_unlock(&rwlock); }

int RWLock::getReadContentionByWriteCount() const {
  return readContentionByWriteCount.load(); // read read atomic value
}

int RWLock::getWriteContentionCount() const {
  return writeContentionCount.load(); // read write atomic value
}

int RWLock::resetReadContentionByWriteCount() {
  return readContentionByWriteCount.exchange(0);
}

int RWLock::resetWriteContentionCount() {
  return writeContentionCount.exchange(0);
}