#include "TaskThreadManager.h"
#include <fstream>
#include <iostream>
#include <thread>

using namespace std;

TaskThreadManager::TaskThreadManager(const std::string &filePath,
                                     TaskQueue &queue)
    : csvFileHandler(new CSVFileHandler(filePath)), taskQueue(&queue),
      stopProducer(false), stopConsumer(false), stopReader(false) {}

TaskThreadManager::~TaskThreadManager() { delete csvFileHandler; }

// execute the task
void TaskThreadManager::executeTask(const Task &task) {
  try {
    // write a row to the CSV file as one execution is done
    csvFileHandler->writeRow({std::to_string(task.id), task.name,
                              task.isCompleted ? "Complete" : "Incomplete"});
    std::cout << "[Tasks] Executed Task ID: " << task.id << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "[Tasks] Error writing to CSV: " << e.what() << std::endl;
  }

  std::cout << "Executing task: " << task.id << " " << task.name << std::endl;
}

// producer thread ------------------------------
// Producer thread function
void *TaskThreadManager::producerThread(void *arg) {
  auto *data = static_cast<std::pair<TaskThreadManager *, int> *>(arg);
  TaskThreadManager *tasks = data->first; // get the tasks object
  int numTasks = data->second;            // get the number of tasks

  for (int i = 1; i <= numTasks; ++i) {
    Task t = {i, "Task_" + std::to_string(i), false};
    tasks->taskQueue->enqueue(t);
    std::cout << "[Producer] Enqueued Task ID: " << t.id << std::endl;
    std::this_thread::sleep_for(
        std::chrono::milliseconds(50)); // delay for 50 milliseconds
  }
  delete data;
  return nullptr;
}

// start the producer thread
void TaskThreadManager::startProducerThread(int numTasks) {
  stopProducer = false; // create task and numTasks
  ThreadData *data = new ThreadData{this, numTasks};
  threadManager.createThread(&TaskThreadManager::producerThread, data, nullptr);
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
    if (t.id == -1) {
      break;
      // debug print ------------------------------
      cout << "Exiting consumer thread" << endl;
    }

    if (manager->taskQueue->dequeue(t)) {
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
  Task endTask = {-1, "Exit", false}; // create a task with id -1
  threadManager.exitThread();
}
//----------------------------------------------

// reader thread ------------------------------
// reader thread function
void *TaskThreadManager::readerThread(void *arg) {
  TaskThreadManager *manager =
      static_cast<TaskThreadManager *>(arg); // get the tasks object

  while (!manager->stopReader.load()) {
    try {
      auto rows = manager->csvFileHandler->readAll();
      cout << "[Reader] CSV Content:" << endl;
      for (const auto &row : rows) {
        for (const auto &cell : row) {
          cout << cell << " ";
        }
        cout << endl;
      }
    } catch (const std::exception &e) {
      cerr << "[Reader] Error reading CSV: " << e.what() << endl;
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
  threadManager.exitThread();
}
//----------------------------------------------

// custom tasks
void TaskThreadManager::customTasks(int producerThreads, int produceCount,
                                    int readerThreads, int consumerThreads,
                                    int writeCount) {
  // average job per thread
  int tasksPerProducer = produceCount / producerThreads;
  int tasksPerConsumer = writeCount / consumerThreads;

  // start producer threads
  for (int i = 0; i < producerThreads; ++i) {
    int startTaskId = i * tasksPerProducer + 1;
    int endTaskId = min((i + 1) * tasksPerProducer, produceCount);
    int numTasks = endTaskId - startTaskId + 1;

    startProducerThread(numTasks);
  }

  // start consumer threads
  for (int i = 0; i < consumerThreads; ++i) {
    int startTaskId = i * tasksPerConsumer + 1;
    int endTaskId = min((i + 1) * tasksPerConsumer, produceCount);
    startConsumerThread();
  }

  // start reader threads
  for (int i = 0; i < readerThreads; ++i) {
    startReaderThread();
  }

  stopConsumerThread(consumerThreads); // stop consumer threads
  // wait for some time to ensure all consumer threads are stopped
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // stopReaderThread
  stopReaderThread();
  // wait for some time to ensure all reader threads are stopped
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // Verify CSV content, check if all tasks are written
  try {
    auto rows = csvFileHandler->readAll();
    if (rows.size() >= static_cast<size_t>(writeCount)) {
      std::cout << "[CustomTasks] All tasks written successfully to CSV!"
                << std::endl;
    } else {
      std::cerr << "[CustomTasks] Missing tasks in CSV. Expected at least "
                << writeCount << ", found " << rows.size() << "." << std::endl;
    }
  } catch (const std::exception &e) {
    std::cerr << "[CustomTasks] Error verifying CSV content: " << e.what()
              << std::endl;
  }
}