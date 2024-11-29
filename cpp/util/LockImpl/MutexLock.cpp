#include "../MutexLock.h"

#include <stdexcept>
using namespace std;

MutexLock::MutexLock() {
  if (pthread_mutex_init(&mutex, nullptr) != 0) {
    throw runtime_error("Mutex initialization failed");
  }
}

void MutexLock::mutexLockOn() { pthread_mutex_lock(&mutex); }

void MutexLock::mutexUnlock() { pthread_mutex_unlock(&mutex); }

MutexLock::~MutexLock() { pthread_mutex_destroy(&mutex); }

void MutexLock::waitOnCondition(pthread_cond_t *cond) {
  pthread_cond_wait(cond, &mutex);
}