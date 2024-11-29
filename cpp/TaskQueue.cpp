#include "TaskQueue.h"
#include <stdexcept>
using namespace std;

// Constructor, initialize the lock type and lock pointer
TaskQueue::TaskQueue(LockType type, void *lock)
    : lockType(type), mutexLock(nullptr), rwLock(nullptr) {
  if (type == LockType::Mutex && lock != nullptr) {
    mutexLock = static_cast<MutexLock *>(lock);
  } else if (type == LockType::RWLock && lock != nullptr) {
    rwLock = static_cast<RWLock *>(lock);
  } else if (type == LockType::Mutex) {
    mutexLock = new MutexLock();
  } else if (type == LockType::RWLock) {
    rwLock = new RWLock();
  } else if (type == LockType::NoLock) {
    // no lock will be used
  } else {
    throw std::invalid_argument("Invalid lock type");
  }
  pthread_cond_init(&cond, nullptr);
}

// Destructor
TaskQueue::~TaskQueue() {
  if (mutexLock != nullptr) {
    delete mutexLock;
  }

  if (rwLock != nullptr) {
    delete rwLock;
  }
}

// lock the queue, based on the lock type
void TaskQueue::lock() {
  if (lockType == LockType::Mutex) {
    mutexLock->mutexLockOn();
  } else if (lockType == LockType::RWLock) {
    rwLock->writeLock();
  } else if (lockType == LockType::NoLock) {
    // no lock will be used
  } else {
    throw std::invalid_argument("Invalid lock type");
  }
}

// unlock the queue, based on the lock type
void TaskQueue::unlock() {
  if (lockType == LockType::Mutex) {
    mutexLock->mutexUnlock();
  } else if (lockType == LockType::RWLock) {
    rwLock->writeUnlock();
  } else if (lockType == LockType::NoLock) {
    // no lock will be used
  } else {
    throw std::invalid_argument("Invalid lock type");
  }
}

// enqueue tasks
void TaskQueue::enqueue(const Task &t) {
  lock();             // lock the queue
  tasksQueue.push(t); // add the task to the queue
  // print for debugging ---------------------------------------------
  std::cout << "[TaskQueue] Enqueued Task ID: " << t.id << ", Name: " << t.name
            << std::endl;
  // ----------------------------------------------------------------------
  pthread_cond_signal(&cond); // signal the condition
  unlock();                   // unlock the queue
}

// dequeue tasks
bool TaskQueue::dequeue(Task &t) {
  lock(); // lock the queue

  if (lockType == LockType::NoLock) {
    if (tasksQueue.empty()) {
      unlock();
      return false; // No task to dequeue
    }
  }

  while (tasksQueue.empty() && lockType != LockType::NoLock) {
    if (lockType == LockType::Mutex && mutexLock) {
      cout << "Queue is empty, waiting for tasks" << endl;
      mutexLock->waitOnCondition(&cond); // wait for the condition
    } else if (lockType == LockType::RWLock && rwLock) {
      rwLock->writeLock(); // Ensure exclusive access for condition wait
      mutexLock->waitOnCondition(&cond);
      rwLock->writeUnlock();
    }
  }

  if (!tasksQueue.empty()) {
    Task frontTask = tasksQueue.front();

    // terminate the dequeue operation if termination signal is received
    if (frontTask.id == -1) { // Check for termination signal
      std::cout << "Received termination signal. Exiting dequeue." << std::endl;
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

    // print for debugging ---------------------------------------------
    cout << "All tasks are removed from the queue" << endl;
    cout << "Task " << t.id << " is removed from the queue" << endl;
    // ----------------------------------------------------------------------
  }

  unlock(); // unlock the queue
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