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
  int ret = pthread_create(thread, nullptr, startRoutine, arg);
  if (ret != 0) {
    throw std::runtime_error("Failed to create thread");
  }
  threadList.push_back(*thread);     // add the thread to the list
  threadStatus[*thread] = "running"; // set the status of the thread
}

// join thread, wait for the thread to finish
void ThreadManager::joinThread(pthread_t thread) {
  if (pthread_join(thread, nullptr) == 0) {
    threadStatus[thread] = "Finished";
  } else {
    throw std::runtime_error("Failed to join thread");
  }
}

// detach thread, thread run independently
void ThreadManager::detachThread(pthread_t thread) {
  if (pthread_detach(thread) == 0) {
    threadStatus[thread] = "Detached";
  } else {
    throw std::runtime_error("Failed to detach thread");
  }
}

// cancel thread,
void ThreadManager::cancelThread(pthread_t thread) {
  if (pthread_cancel(thread) == 0) {
    threadStatus[thread] = "Cancelled";
  } else {
    throw std::runtime_error("Failed to cancel thread");
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