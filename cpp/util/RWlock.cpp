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
    cout << "[DEBUG] Read lock contention detected. Current count: "
         << readContentionByWriteCount << endl;
    pthread_rwlock_rdlock(&rwlock); // Block until read lock is acquired
    cout << "[DEBUG] Read lock acquired after contention." << endl;
  } else {
    cout << "[DEBUG] Read lock acquired immediately." << endl;
  }
}

void RWLock::writeLock() {
  if (pthread_rwlock_trywrlock(&rwlock) != 0) {
    writeContentionCount++;
    cout << "[DEBUG] Write lock contention detected. Current count: "
         << writeContentionCount << endl;
    pthread_rwlock_wrlock(&rwlock); // Block until write lock is acquired
    cout << "[DEBUG] Write lock acquired after contention." << endl;
  } else {
    cout << "[DEBUG] Write lock acquired immediately." << endl;
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