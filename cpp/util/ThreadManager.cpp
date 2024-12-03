#include "ThreadManager.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

ThreadManager::ThreadManager() {}

ThreadManager::~ThreadManager() {}

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

  cout << "[ThreadManager::createThread] Thread " << *threadPtr
       << " created successfully." << endl;
}

// join thread, wait for the thread to finish
void ThreadManager::joinAllThreads() {
  for (auto &thread : threadList) {
    std::cout << "[joinAllThreads] Waiting for thread: " << thread << std::endl;
    int result = pthread_join(thread, nullptr);
    if (result == 0) {
      threadStatus[thread] = "Finished";
      std::cout << "[joinAllThreads] Thread " << thread << " finished."
                << std::endl;
    } else {
      std::cerr << "[joinAllThreads] Error joining thread: " << thread
                << ", error code: " << result << std::endl;
    }
  }
  threadList.clear();
}
// exit thread
// void ThreadManager::exitThread() { pthread_exit(NULL); }

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

void ThreadManager::recordThreadRunTime(pthread_t thread, long time) {
  threadRunTime[thread] += time;
}

void ThreadManager::incrementBlockCount(pthread_t thread) {
  threadBlockCount[thread]++;
}

void ThreadManager::exportThreadMetrics(const std::string &filePath) {
  std::ofstream file(filePath);
  file << "ThreadID,Status,RunTime(us),BlockCount\n";
  for (const auto &thread : threadList) {
    file << thread << "," << threadStatus[thread] << ","
         << threadRunTime[thread] << "," << threadBlockCount[thread] << "\n";
  }
  file.close();
}