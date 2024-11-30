#include "ThreadManager.h"
#include <iostream>
#include <stdexcept>

ThreadManager::ThreadManager() {}

ThreadManager::~ThreadManager() {
  for (auto &thread : threadList) {
    pthread_cancel(thread);
  }
}

//------------------Thread management------------------
void ThreadManager::createThread(void *(*startRoutine)(void *), void *arg,
                                 pthread_t *thread) {
  if (arg != nullptr) {
    int *data = static_cast<int *>(arg);
    if (*data < 0) {
      throw std::invalid_argument(
          "Invalid data for thread: negative value detected!");
    }
  }

  pthread_t localThread;
  pthread_t *threadPtr = thread ? thread : &localThread;

  int result = pthread_create(threadPtr, nullptr, startRoutine, arg);
  if (result != 0) {
    throw std::runtime_error("Failed to create thread");
  }

  threadList.push_back(*threadPtr);
  threadStatus[*threadPtr] = "Running";
}

// join thread, wait for the thread to finish
void ThreadManager::joinThread(pthread_t thread) {
  if (pthread_join(thread, nullptr) == 0) {
    threadStatus[thread] = "Finished";
  } else {
    throw std::runtime_error("Failed to join thread");
  }
}

// exit thread
void ThreadManager::exitThread() { pthread_exit(NULL); }

//------------------Thread status------------------

// get the status of a single thread
std::string ThreadManager::getThreadStatus(pthread_t thread) {
  auto it = threadStatus.find(thread);
  if (it != threadStatus.end()) {
    return it->second;
  } else {
    throw std::runtime_error("Thread not found in threadStatus");
  }
}

// get the status of all threads
std::unordered_map<pthread_t, std::string> ThreadManager::getThreadStatus() {
  return threadStatus;
}

// print the status of all threads
// this is for debugging purpose
void ThreadManager::printThreadStatus() {
  for (const auto &entry : threadStatus) {
    std::cout << "Thread " << entry.first << ": " << entry.second << std::endl;
  }
}