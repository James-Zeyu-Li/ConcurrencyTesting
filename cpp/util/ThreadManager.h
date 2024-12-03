#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <chrono>
#include <pthread.h>
#include <string>
#include <unordered_map>
#include <vector>

class ThreadManager {
private:
  std::vector<pthread_t> threadList;
  std::unordered_map<pthread_t, std::string> threadStatus;
  std::unordered_map<pthread_t, long>
      threadRunTime; // Thread runtime in microseconds
  std::unordered_map<pthread_t, int> threadBlockCount; // Thread block count

public:
  ThreadManager();
  ~ThreadManager();

  void createThread(void *(*startRoutine)(void *), void *arg,
                    pthread_t *thread);
  void joinAllThreads(); // Join the thread, wait for the thread to finish

  // void exitThread(); // Exit the thread

  // get thread status for single thread
  std::string getThreadStatus(pthread_t thread);

  // get thread status for all threads
  std::unordered_map<pthread_t, std::string> getThreadStatus();

  // print the status of all threads
  void printThreadStatus();

  // record the runtime of a thread, benchmarking
  void recordThreadRunTime(pthread_t thread, long time);
  void incrementBlockCount(pthread_t thread);
  void exportThreadMetrics(const std::string &filePath);
};

#endif // THREADMANAGER_H