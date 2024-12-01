#include "ProducerConsumerConcurrentIO.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std;

ProducerConsumerConcurrentIO::ProducerConsumerConcurrentIO(
    const string &filePath, TaskQueue &queue)
    : csvHandler(make_unique<CSVHandler>(filePath, LockType::Mutex)),
      taskQueue(&queue), stopProducer(false), stopConsumer(false),
      stopReader(false) {}

// get the CSV content, for testing purposes, read all rows
vector<vector<string>> ProducerConsumerConcurrentIO::getCSVContent() {
  try {
    return csvHandler->readAll();
  } catch (const exception &e) {
    cerr << "Error reading CSV content: " << e.what() << endl;
    throw;
  }
} //----------------------------------------------

ProducerConsumerConcurrentIO::~ProducerConsumerConcurrentIO() = default;

// execute the task
void ProducerConsumerConcurrentIO::executeTask(const Task &task) {
  try {
    cout << "[executeTask] Attempting to write Task ID: " << task.id << endl;

    // write a row to the CSV file as one execution is done
    csvHandler->writeRow({to_string(task.id), task.name,
                          task.isCompleted ? "Complete" : "Incomplete"});
    cout << "Executed Task ID: " << task.id << ::endl;
  } catch (const ::exception &e) {
    cerr << " Error writing to CSV: " << e.what() << ::endl;
  }

  cout << "Executing task: " << task.id << " " << task.name << ::endl;
}

// Initiate Global Counter
atomic<int> ProducerConsumerConcurrentIO::globalTaskCounter{1};

// producer thread ------------------------------
// Producer thread function, 100ms sleep between each task
void *ProducerConsumerConcurrentIO::producerThread(void *arg) {
  auto *data = static_cast<ThreadData *>(arg);
  if (!data) {
    cerr << "Error: ThreadData is null in producerThread." << endl;
    return nullptr;
  }

  ProducerConsumerConcurrentIO *manager = data->manager;
  if (!manager) {
    cerr << "Error: ProducerConsumerConcurrentIO is null in producerThread."
         << endl;
    delete data;
    return nullptr;
  }

  int taskCount = data->taskCount;
  cout << "Producer thread started. Task count: " << taskCount << endl;

  for (int i = 1; i <= taskCount; ++i) {
    if (manager->stopProducer) {
      cout << "Producer thread stopping due to stopProducer flag." << endl;
      break;
    }

    int taskID = globalTaskCounter.fetch_add(1);
    Task task{taskID, "Task_" + to_string(taskID), false};

    try {
      cout << "[producerThread] Enqueuing Task ID: " << taskID << endl;
      manager->taskQueue->enqueue(task);
    } catch (const exception &e) {
      cerr << "Error in enqueue: " << e.what() << endl;
      break;
    }

    this_thread::sleep_for(chrono::milliseconds(100));
  }

  cout << "Producer thread finished." << endl;
  delete data;
  return nullptr;
}

// start the producer thread
void ProducerConsumerConcurrentIO::startProducerThread(int numTasks) {
  if (numTasks <= 0) {
    throw invalid_argument("Number of tasks must be positive.");
  }

  stopProducer = false;
  auto *data = new ThreadData{this, numTasks};
  pthread_t threadHandle;

  try {
    threadManager.createThread(&ProducerConsumerConcurrentIO::producerThread,
                               data, &threadHandle);
  } catch (const exception &e) {
    cerr << "Error starting producer thread: " << e.what() << endl;
    delete data; // delete the data
    throw;
  }
}

// stop the producer thread
void ProducerConsumerConcurrentIO::stopProducerThread() {
  stopProducer = true;
  cout << "[stopProducerThread] Setting stopProducer to true." << endl;
  // cout << "joinAllThreads" << endl;
  // threadManager.joinAllThreads();
  // cout << "[stopProducerThread] Producer threads exited." << endl;
}
//----------------------------------------------

// consumer thread ------------------------------
// consumer thread function, 100ms sleep between each task
void *ProducerConsumerConcurrentIO::consumerThread(void *arg) {
  ProducerConsumerConcurrentIO *manager =
      static_cast<ProducerConsumerConcurrentIO *>(arg);
  std::cout << "[consumerThread] Started, waiting for tasks..." << std::endl;

  while (!manager->stopConsumer) {
    Task t;
    if (manager->taskQueue->dequeue(t)) {
      if (t.id == -1) {
        cout << "Consumer received termination signal." << endl;
        break;
      }
      cout << "[consumerThread] Processing Task ID: " << t.id << endl;
      manager->executeTask(t);
    } else {
      // check if producer has stopped
      if (manager->stopProducer && manager->taskQueue->isEmpty()) {
        cout << "No more tasks and producer has stopped. Exiting consumer "
                "thread."
             << endl;
        break;
      }
      cout << "[consumerThread] Waiting for tasks..." << endl;
      this_thread::sleep_for(chrono::milliseconds(100));
    }
  }
  cout << "[consumerThread] Exiting normally." << endl;
  return nullptr;
}

// start the consumer thread
void ProducerConsumerConcurrentIO::startConsumerThread() {
  stopConsumer = false;
  threadManager.createThread(&ProducerConsumerConcurrentIO::consumerThread,
                             this, nullptr);
}

// stop the consumer thread
void ProducerConsumerConcurrentIO::stopConsumerThread(int consumerTaskCount) {
  stopConsumer = true;
  cout << "[stopConsumerThread] Setting stopConsumer to true." << endl;
  while (consumerTaskCount-- > 0) {
    Task endTask = {-1, "Exit", false}; // create a task with id -1
    taskQueue->enqueue(endTask);
  }
  cout << "[stopConsumerThread] Exit Task Enqueued." << endl;
}
//----------------------------------------------

// reader thread ------------------------------
// reader thread function
void *ProducerConsumerConcurrentIO::readerThread(void *arg) {
  ProducerConsumerConcurrentIO *manager =
      static_cast<ProducerConsumerConcurrentIO *>(arg);

  while (!manager->stopReader) {
    try {
      this_thread::sleep_for(chrono::milliseconds(500)); // Adjust timing
      auto rows = manager->csvHandler->readAll();
      cout << "Reader Thread CSV Content:" << endl;
      for (const auto &row : rows) {
        for (const auto &cell : row) {
          cout << cell << " ";
        }
        cout << endl;
      }

      // check if all tasks are read, then stop the reader
      if (rows.size() >= static_cast<size_t>(manager->taskQueue->queueSize())) {
        cout << "  Reader   All tasks read from CSV" << endl;
        manager->stopReader = true; // Stop the reader
        break;
      }

    } catch (const exception &e) {
      cerr << "Reader error: " << e.what() << endl;
    }
  }

  return nullptr;
}

// start the reader thread
void ProducerConsumerConcurrentIO::startReaderThread() {
  stopReader = false;
  threadManager.createThread(&ProducerConsumerConcurrentIO::readerThread, this,
                             nullptr);
}

// stop the reader thread
void ProducerConsumerConcurrentIO::stopReaderThread() {
  stopReader = true;
  cout << "  Reader   Stopping Reader Thread" << endl;
}
//----------------------------------------------

// custom tasks
void ProducerConsumerConcurrentIO::customTasks(int producerThreads,
                                               int produceCount,
                                               int readerThreads,
                                               int consumerThreads,
                                               int writeCount) {
  // average job per thread
  int tasksPerProducer = produceCount / producerThreads;
  int remainingTasks = produceCount % producerThreads;

  for (int i = 0; i < producerThreads; ++i) {
    int numTasks = tasksPerProducer + (i < remainingTasks ? 1 : 0);
    cout << "Producer thread " << i << " assigned " << numTasks << " tasks.\n";

    // task assignment to producer threads
    startProducerThread(numTasks);
  }
  // activate consumer thread
  for (int i = 0; i < consumerThreads; ++i) {
    startConsumerThread();
  }

  // activate reader thread
  for (int i = 0; i < readerThreads; ++i) {
    startReaderThread();
  }

  // wait for the producer threads to finish
  this_thread::sleep_for(chrono::seconds(15));

  // stop the producer threads
  stopProducerThread();

  // if the test is successful, stop the consumer and reader threads
  stopConsumerThread(consumerThreads);

  this_thread::sleep_for(chrono::seconds(10));

  cout << "[test] Joining all threads..." << endl;

  stopReaderThread();
  this_thread::sleep_for(chrono::seconds(5));

  threadManager.joinAllThreads();

  // Verify CSV content, check if all tasks are written
  try {
    auto rows = csvHandler->readAll();
    if (rows.size() >= static_cast<size_t>(writeCount)) {
      cout << " All tasks written successfully to CSV!" << endl;
    } else {
      cerr << "Missing tasks in CSV. Expected at least " << writeCount
           << ", found " << rows.size() << "." << endl;
    }
  } catch (const ::exception &e) {
    cerr << " Error verifying CSV content: " << e.what() << endl;
  }
}