#include "MutexLock.h"

#include <stdexcept>
using namespace std;

MutexLock::MutexLock() {
  if (pthread_mutex_init(&mutex, nullptr) != 0) {
    throw runtime_error("Mutex initialization failed");
  }
}

MutexLock::~MutexLock() { pthread_mutex_destroy(&mutex); }

void MutexLock::mutexLockOn() {
  if (pthread_mutex_trylock(&mutex) != 0) {
    mutexContentionCount++; // if lock fail, increase count
  }
  pthread_mutex_lock(&mutex);
}

void MutexLock::mutexUnlock() { pthread_mutex_unlock(&mutex); }

void MutexLock::waitOnCondition(pthread_cond_t *cond) {
  pthread_cond_wait(cond, &mutex);
}

// get the contention count
int MutexLock::getContentionCount() const { return mutexContentionCount; }
