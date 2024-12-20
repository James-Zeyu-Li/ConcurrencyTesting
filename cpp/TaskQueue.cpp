#include "TaskQueue.h"
#include <stdexcept>
using namespace std;

// Constructor, initialize the lock type and lock pointer
TaskQueue::TaskQueue(LockType type, void *lock)
    : lockType(type), mutexLock(nullptr), rwLock(nullptr),
      isExternalLock(lock != nullptr) {
  if (type == LockType::Mutex && lock != nullptr) {
    mutexLock = static_cast<MutexLock *>(lock);
  } else if (type == LockType::RWLock && lock != nullptr) {
    rwLock = static_cast<RWLock *>(lock);
  } else if (type == LockType::Mutex) {
    mutexLock = new MutexLock();
    if (!mutexLock) {
      throw runtime_error("Failed to allocate MutexLock");
    }
  } else if (type == LockType::RWLock) {
    rwLock = new RWLock();
    if (!rwLock) {
      throw runtime_error("Failed to allocate RWLock");
    }
  } else {
    throw invalid_argument("Invalid lock type");
  }

  if (pthread_cond_init(&cond, nullptr) != 0) {
    throw runtime_error("Failed to initialize condition variable");
  }

  // **Add initialization of condMutex**
  if (pthread_mutex_init(&queueMutex, nullptr) != 0) {
    throw runtime_error("Failed to initialize condition mutex");
  }
}

// Destructor
TaskQueue::~TaskQueue() {
  if (!isExternalLock) {
    if (mutexLock != nullptr) {
      delete mutexLock;
      mutexLock = nullptr;
    }
    if (rwLock != nullptr) {
      delete rwLock;
      rwLock = nullptr;
    }
  }
  pthread_cond_destroy(&cond);
  pthread_mutex_destroy(&queueMutex);
}

void TaskQueue::lock() {
  if (lockType == LockType::Mutex) {
    mutexLock->mutexLockOn();
  } else if (lockType == LockType::RWLock) {
    rwLock->writeLock();
  } else {
    throw invalid_argument("Invalid lock type");
  }
}

// unlock the queue, based on the lock type
void TaskQueue::unlock() {
  if (lockType == LockType::Mutex) {
    mutexLock->mutexUnlock();
  } else if (lockType == LockType::RWLock) {
    rwLock->writeUnlock();
  } else {
    throw invalid_argument("Invalid lock type");
  }
}

// enqueue tasks
void TaskQueue::enqueue(const Task &t) {
  auto start = chrono::high_resolution_clock::now();

  pthread_mutex_lock(&queueMutex); // lock the condition mutex
  lock();
  tasksQueue.push(t); // add the task to the queue
  int currentLength = tasksQueue.size();
  maxQueueLength = std::max(maxQueueLength.load(), currentLength);
  unlock();                          // unlock the queue
  pthread_mutex_unlock(&queueMutex); // unlock the queue]
  pthread_cond_signal(&cond);        // Notify a waiting thread

  // Benchmark Tools, time calculation
  auto end = chrono::high_resolution_clock::now();
  long timeTaken =
      chrono::duration_cast<chrono::microseconds>(end - start).count();
  totalEnqueueTime += timeTaken;
  enqueueCount++;
  maxEnqueueTime = max(maxEnqueueTime.load(), timeTaken);
  minEnqueueTime = min(minEnqueueTime.load(), timeTaken);
}

// dequeue tasks
bool TaskQueue::dequeue(Task &t) {

  pthread_mutex_lock(&queueMutex); // Lock condition mutex

  while (tasksQueue.empty()) { // Wait until there is a task
    pthread_cond_wait(&cond, &queueMutex);
  }

  pthread_mutex_unlock(&queueMutex); // Unlock condition mutex
  lock();                            // lock the queue

  if (!tasksQueue.empty()) {
    auto start = chrono::high_resolution_clock::now();

    Task frontTask = tasksQueue.front();

    // terminate the dequeue operation if termination signal is received
    if (frontTask.id == -1) { // Check for termination signal
      cout << "Received termination signal. Exiting dequeue." << endl;
      tasksQueue.pop(); // Remove the termination signal
      unlock();
      return false; // Indicate that termination signal was received
    }

    t = frontTask;    // get the task from the front
    tasksQueue.pop(); // remove the task from the queue
    // print for debugging
    // ---------------------------------------------
    cout << "Task " << t.id << " is removed from the queue" << endl;
    // ----------------------------------------------------------------------

    unlock(); // unlock the queue

    // Benchmark Tools, time calculation, end time
    auto end = chrono::high_resolution_clock::now();
    long timeTaken =
        chrono::duration_cast<chrono::microseconds>(end - start).count();
    totalDequeueTime += timeTaken;
    dequeueCount++;
    maxDequeueTime = max(maxDequeueTime.load(), timeTaken);
    minDequeueTime = min(minDequeueTime.load(), timeTaken);
    // ----------------------------------------------

    return true;
  }
  unlock(); // unlock the queue
  return false;
}

// dequeue all tasks
void TaskQueue::dequeueAll() {
  lock(); // lock the queue
  while (!tasksQueue.empty()) {
    Task t = tasksQueue.front();
    tasksQueue.pop(); // remove the task from the queue
  }
  unlock(); // unlock the queue
  cout << "All tasks are removed from the queue" << endl;
}

// check if the queue is empty
bool TaskQueue::isEmpty() {
  lock(); // lock the queue
  bool empty = tasksQueue.empty();
  unlock(); // unlock the queue
  return empty;
}

// get the size of the queue
int TaskQueue::queueSize() {
  lock(); // lock the queue
  int size = tasksQueue.size();
  unlock(); // unlock the queue
  return size;
}

// Benchmark metrics
long TaskQueue::getTotalEnqueueTime() const { return totalEnqueueTime; }
long TaskQueue::getTotalDequeueTime() const { return totalDequeueTime; }
double TaskQueue::getAverageEnqueueTime() const {
  return enqueueCount > 0 ? static_cast<double>(totalEnqueueTime) / enqueueCount
                          : 0;
}
double TaskQueue::getAverageDequeueTime() const {
  return dequeueCount > 0 ? static_cast<double>(totalDequeueTime) / dequeueCount
                          : 0;
}
long TaskQueue::getMaxEnqueueTime() const { return maxEnqueueTime; }
long TaskQueue::getMinEnqueueTime() const { return minEnqueueTime; }
long TaskQueue::getMaxDequeueTime() const { return maxDequeueTime; }
long TaskQueue::getMinDequeueTime() const { return minDequeueTime; }

int TaskQueue::getBlockCount() const {
  if (lockType == LockType::Mutex) {
    return mutexLock ? mutexLock->getContentionCount() : 0;
  } else if (lockType == LockType::RWLock) {
    return rwLock ? rwLock->getWriteContentionCount() +
                        rwLock->getReadContentionByWriteCount()
                  : 0;
  }
  return 0;
}

// Lock management
MutexLock *TaskQueue::getMutexLock() const { return mutexLock; }
RWLock *TaskQueue::getRWLock() const { return rwLock; }

int TaskQueue::getMaxQueueLength() const { return maxQueueLength; }