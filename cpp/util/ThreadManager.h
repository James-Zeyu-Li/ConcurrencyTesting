#ifndef THREADMANAGER_H
#define THREADMANAGER_H

#include <pthread.h>
#include <unordered_map>

#include <vector>

class ThreadManager {
private:
  std::vector<pthread_t> threadList;
  std::unordered_map<pthread_t, std::string> threadStatus;

public:
  ThreadManager(/* args */);
  ~ThreadManager();

  void createThread(void *(*startRoutine)(void *), void *arg,
                    pthread_t *thread);
  void joinThread(
      pthread_t thread); // Join the thread, wait for the thread to finish

  void
  detachThread(pthread_t thread); // Detach the thread, thread run independently

  void exitThread();                   // Exit the thread
  void cancelThread(pthread_t thread); // Cancel the thread

  // get thread status for single thread
  std::string getThreadStatus(pthread_t thread);

  // get thread status for all threads
  std::unordered_map<pthread_t, std::string> getThreadStatus();

  // print the status of all threads
  void printThreadStatus();
};

#endif // THREADMANAGER_H