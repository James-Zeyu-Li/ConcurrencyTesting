#ifndef PRODUCERCONSUMERCONCURRENTIO_H
#define PRODUCERCONSUMERCONCURRENTIO_H

#include "CSVHandler.h"
#include "TaskQueue.h"
#include "util/ThreadManager.h"
#include <chrono>
#include <iostream>
#include <string>

class ProducerConsumerConcurrentIO {

private:
  struct ThreadData {
    ProducerConsumerConcurrentIO *manager;
    int taskCount; // number of tasks to produce
  };

  std::unique_ptr<CSVHandler> csvHandler;
  TaskQueue *taskQueue;
  ThreadManager threadManager;

  std::atomic<bool> stopProducer;
  std::atomic<bool> stopConsumer;
  std::atomic<bool> stopReader;

  // static thread functions
  static void *producerThread(void *arg);
  static void *consumerThread(void *arg);
  static void *readerThread(void *arg);

  void executeTask(const Task &task);
  static std::atomic<int> globalTaskCounter;

public:
  ProducerConsumerConcurrentIO(const std::string &filePath, TaskQueue &queue);
  ~ProducerConsumerConcurrentIO();

  void startProducerThread(int numTasks);
  void stopProducerThread();

  void startConsumerThread();
  void stopConsumerThread(int consumerTaskCount);

  void startReaderThread();
  void stopReaderThread();

  void customTasks(int producerThreads, int produceCount, int readerThreads,
                   int consumerThreads, int writeCount);

public:
  // get the CSV content, for testing purposes
  std::vector<std::vector<std::string>> getCSVContent();
};

#endif // PRODUCERCONSUMERCONCURRENTIO_H