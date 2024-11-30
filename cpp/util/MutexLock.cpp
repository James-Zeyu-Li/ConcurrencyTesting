#include "MutexLock.h"
#include <iostream>

#include <stdexcept>
using namespace std;

MutexLock::MutexLock() : mutexContentionCount(0) {
  if (pthread_mutex_init(&mutex, nullptr) != 0) {
    throw runtime_error("Mutex initialization failed");
  }
}

MutexLock::~MutexLock() { pthread_mutex_destroy(&mutex); }

void MutexLock::mutexLockOn() {
  if (pthread_mutex_trylock(&mutex) != 0) {
    mutexContentionCount++;
    pthread_mutex_lock(&mutex);
  }
  cout << "thread " << pthread_self() << " Mutex lock acquired." << endl;
}

void MutexLock::mutexUnlock() { pthread_mutex_unlock(&mutex); }

void MutexLock::waitOnCondition(pthread_cond_t *cond) {
  pthread_cond_wait(cond, &mutex);
}

int MutexLock::getContentionCount() const {
  return mutexContentionCount.load();
}

int MutexLock::resetContentionCount() {
  return mutexContentionCount.exchange(0);
}