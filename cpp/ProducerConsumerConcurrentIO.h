// ProducerConsumerConcurrentIO.h
#ifndef PRODUCER_CONSUMER_CONCURRENT_IO_H
#define PRODUCER_CONSUMER_CONCURRENT_IO_H

#include "CSVHandler.h"
#include "TaskQueue.h" // Include this to use Task struct
#include "util/LockType.h"
#include "util/MutexLock.h"
#include "util/RWLock.h"
#include "util/ThreadManager.h"

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct ThreadData {
  class ProducerConsumerConcurrentIO *manager;
  int taskCount;
};

class ProducerConsumerConcurrentIO {
public:
  ProducerConsumerConcurrentIO(const std::string &filePath,
                               std::shared_ptr<TaskQueue> queue,
                               LockType lockType);
  ~ProducerConsumerConcurrentIO();

  void customTasks(int producerThreads, int produceCount, int readerThreads,
                   int consumerThreads, int writeCount);
  void executeTask(const Task &task);
  std::vector<std::vector<std::string>> getCSVContent();

  // Getters
  static int getGlobalTaskCounter();
  int getTasksCompleted() const;
  bool isReadCompleted() const;
  std::shared_ptr<TaskQueue> getTaskQueue() const;
  const std::map<pthread_t, int> &getThreadTaskCount() const;
  const std::map<pthread_t, long long> &getThreadActiveTime() const;
  const std::vector<pthread_t> &getProducerThreadIds() const;
  const std::vector<pthread_t> &getConsumerThreadIds() const;
  const std::vector<pthread_t> &getReaderThreadIds() const;
  void resetGlobalTaskCounter();
  ThreadManager &getThreadManager();

private:
  // Thread functions
  static void *producerThread(void *arg);
  static void *consumerThread(void *arg);
  static void *readerThread(void *arg);

  void startProducerThread(int numTasks);
  void stopProducerThread();
  void startConsumerThread();
  void stopConsumerThread(int consumerTaskCount);
  void startReaderThread();
  void stopReaderThread();

  // Statistics collection
  std::map<pthread_t, int> threadTaskCount;
  std::map<pthread_t, long long> threadActiveTime;

  // Synchronization
  std::shared_ptr<TaskQueue> taskQueue;
  std::unique_ptr<CSVHandler> csvHandler;
  ThreadManager threadManager;

  // Control flags
  std::atomic<bool> stopProducer;
  std::atomic<bool> stopConsumer;
  std::atomic<bool> stopReader;
  std::atomic<bool> readCompleted;
  std::atomic<int> tasksCompleted;

  // Thread IDs
  std::vector<pthread_t> producerThreadIds;
  std::vector<pthread_t> consumerThreadIds;
  std::vector<pthread_t> readerThreadIds;

  // Global task counter
  static std::atomic<int> globalTaskCounter;
};

#endif // PRODUCER_CONSUMER_CONCURRENT_IO_H