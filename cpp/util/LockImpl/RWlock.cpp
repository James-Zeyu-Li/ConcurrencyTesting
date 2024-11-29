#include "../RWLock.h"

#include <stdexcept>  // For exception handling
using namespace std;

RWLock::RWLock() {
  if (pthread_rwlock_init(&rwlock, nullptr) != 0) {
    throw runtime_error("Failed to initialize read-write lock");
  }
}

RWLock::~RWLock() { pthread_rwlock_destroy(&rwlock); }

void RWLock::readLock() { pthread_rwlock_rdlock(&rwlock); }

void RWLock::writeLock() { pthread_rwlock_wrlock(&rwlock); }

void RWLock::readUnlock() { pthread_rwlock_unlock(&rwlock); }

void RWLock::writeUnlock() { pthread_rwlock_unlock(&rwlock); }

