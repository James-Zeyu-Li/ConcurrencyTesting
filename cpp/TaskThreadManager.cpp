#include "TaskThreadManager.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std;

TaskThreadManager::TaskThreadManager(const string &filePath, TaskQueue &queue)
    : csvHandler(make_unique<CSVHandler>(filePath, LockType::Mutex)),
      taskQueue(&queue), stopProducer(false), stopConsumer(false),
      stopReader(false) {}

// get the CSV content, for testing purposes, read all rows
vector<vector<string>> TaskThreadManager::getCSVContent() {
  try {
    return csvHandler->readAll();
  } catch (const exception &e) {
    cerr << "Error reading CSV content: " << e.what() << endl;
    throw;
  }
} //----------------------------------------------

TaskThreadManager::~TaskThreadManager() = default;

// execute the task
void TaskThreadManager::executeTask(const Task &task) {
  try {
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
std::atomic<int> TaskThreadManager::globalTaskCounter{1};

// producer thread ------------------------------
// Producer thread function
void *TaskThreadManager::producerThread(void *arg) {
  auto *data = static_cast<ThreadData *>(arg);
  if (!data) {
    std::cerr << "Error: ThreadData is null in producerThread." << std::endl;
    return nullptr;
  }

  TaskThreadManager *manager = data->manager;
  if (!manager) {
    std::cerr << "Error: TaskThreadManager is null in producerThread."
              << std::endl;
    delete data;
    return nullptr;
  }

  int taskCount = data->taskCount;
  std::cout << "Producer thread started. Task count: " << taskCount
            << std::endl;

  for (int i = 1; i <= taskCount; ++i) {
    if (manager->stopProducer) {
      std::cout << "Producer thread stopping due to stopProducer flag."
                << std::endl;
      break;
    }

    int taskID = globalTaskCounter.fetch_add(1);
    Task task{taskID, "Task_" + std::to_string(taskID), false};

    try {
      manager->taskQueue->enqueue(task);
    } catch (const std::exception &e) {
      std::cerr << "Error in enqueue: " << e.what() << std::endl;
      break;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::cout << "Producer thread finished." << std::endl;
  delete data;
  return nullptr;
}

// start the producer thread
void TaskThreadManager::startProducerThread(int numTasks) {
  if (numTasks <= 0) {
    throw std::invalid_argument("Number of tasks must be positive.");
  }

  stopProducer = false;
  auto *data = new ThreadData{this, numTasks};
  pthread_t threadHandle;

  try {
    threadManager.createThread(&TaskThreadManager::producerThread, data,
                               &threadHandle);
  } catch (const std::exception &e) {
    std::cerr << "Error starting producer thread: " << e.what() << std::endl;
    delete data; // delete the data
    throw;
  }
}

// stop the producer thread
void TaskThreadManager::stopProducerThread() {
  stopProducer = true;
  threadManager.exitThread();
}
//----------------------------------------------

// consumer thread ------------------------------
// consumer thread function
void *TaskThreadManager::consumerThread(void *arg) {
  TaskThreadManager *manager = static_cast<TaskThreadManager *>(arg);

  while (!manager->stopConsumer) {
    Task t;
    if (manager->taskQueue->dequeue(t)) {
      if (t.id == -1) {
        cout << "customer Received Termination Signal" << endl;
        break;
      }
      manager->executeTask(t);
    }
  }
  return nullptr;
}

// start the consumer thread
void TaskThreadManager::startConsumerThread() {
  stopConsumer = false;
  threadManager.createThread(&TaskThreadManager::consumerThread, this, nullptr);
}

// stop the consumer thread
void TaskThreadManager::stopConsumerThread(int consumerTaskCount) {
  stopConsumer = true;
  for (int i = 0; i < consumerTaskCount; ++i) {
    Task endTask = {-1, "Exit", false}; // create a task with id -1
    taskQueue->enqueue(endTask);
  }
  cout << "  Consumer Enqueued Exit Task" << endl;
}
//----------------------------------------------

// reader thread ------------------------------
// reader thread function
void *TaskThreadManager::readerThread(void *arg) {
  TaskThreadManager *manager =
      static_cast<TaskThreadManager *>(arg); // get the tasks object

  while (!manager->stopReader.load()) {
    try {
      auto rows = manager->csvHandler->readAll();
      cout << "  Reader    CSV Content:" << endl;
      for (const auto &row : rows) {
        for (const auto &cell : row) {
          cout << cell << " ";
        }
        cout << endl;
      }
    } catch (const ::exception &e) {
      cerr << "  Reader    Error reading CSV: " << e.what() << endl;
    }
    this_thread::sleep_for(chrono::seconds(5));
  }

  return nullptr;
}

// start the reader thread
void TaskThreadManager::startReaderThread() {
  stopReader = false;
  threadManager.createThread(&TaskThreadManager::readerThread, this, nullptr);
}

// stop the reader thread
void TaskThreadManager::stopReaderThread() {
  stopReader = true;
  cout << "  Reader   Stopping Reader Thread" << endl;
}
//----------------------------------------------

// custom tasks
void TaskThreadManager::customTasks(int producerThreads, int produceCount,
                                    int readerThreads, int consumerThreads,
                                    int writeCount) {
  // average job per thread
  int tasksPerProducer = produceCount / producerThreads;

  // start producer threads
  for (int i = 0; i < producerThreads; ++i) {
    int startTaskId = i * tasksPerProducer + 1;
    int endTaskId = min((i + 1) * tasksPerProducer, produceCount);
    int numTasks = endTaskId - startTaskId + 1;

    startProducerThread(numTasks);
  }

  // start consumer threads
  for (int i = 0; i < consumerThreads; ++i) {
    startConsumerThread();
  }

  // start reader threads
  for (int i = 0; i < readerThreads; ++i) {
    startReaderThread();
  }

  stopConsumerThread(consumerThreads); // stop consumer threads
  // wait for some time to ensure all consumer threads are stopped
  this_thread::sleep_for(::chrono::seconds(2));

  // stopReaderThread
  stopReaderThread();
  // wait for some time to ensure all reader threads are stopped
  this_thread::sleep_for(::chrono::seconds(2));

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