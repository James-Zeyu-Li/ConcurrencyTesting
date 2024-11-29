#include "../TaskQueue.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

// Helper function to create a task
Task createTask(int id, const string &name) {
  Task t;
  t.id = id;
  t.name = name;
  return t;
}

// Test TaskQueue basic functionality
void testTaskQueueBasic() {
  cout << "===== Test TaskQueue Basic Functionality =====" << endl;
  TaskQueue queue(LockType::Mutex, nullptr);

  // Test enqueue
  Task t1 = createTask(1, "Task1");
  Task t2 = createTask(2, "Task2");
  queue.enqueue(t1);
  queue.enqueue(t2);

  assert(!queue.isEmpty());
  assert(queue.queueSize() == 2);

  // Test dequeue
  Task dequeuedTask;
  assert(queue.dequeue(dequeuedTask));
  assert(dequeuedTask.id == 1);
  assert(dequeuedTask.name == "Task1");

  assert(queue.dequeue(dequeuedTask));
  assert(dequeuedTask.id == 2);
  assert(dequeuedTask.name == "Task2");

  assert(queue.isEmpty());
  assert(queue.queueSize() == 0);
  cout << "Basic functionality passed!" << endl;
}

// Test TaskQueue termination signal
void testTaskQueueTermination() {
  cout << "===== Test TaskQueue Termination Signal =====" << endl;
  TaskQueue queue(LockType::Mutex, nullptr);

  // Add termination signal
  Task terminationSignal = createTask(-1, "Terminate");
  queue.enqueue(terminationSignal);

  Task dequeuedTask;
  assert(
      !queue.dequeue(dequeuedTask)); // Termination signal should stop dequeue
  cout << "Termination signal handled correctly!" << endl;
}

// Test TaskQueue with multiple threads
void testTaskQueueConcurrency() {
  cout << "===== Test TaskQueue Concurrency =====" << endl;
  TaskQueue queue(LockType::Mutex, nullptr);

  // Producer thread
  auto producer = [&queue]() {
    for (int i = 1; i <= 10; ++i) {
      queue.enqueue(createTask(i, "Task" + to_string(i)));
      this_thread::sleep_for(chrono::milliseconds(50));
    }
  };

  // Consumer thread
  auto consumer = [&queue]() {
    for (int i = 1; i <= 10; ++i) {
      Task dequeuedTask;
      if (queue.dequeue(dequeuedTask)) {
        cout << "Consumed: " << dequeuedTask.id << " - " << dequeuedTask.name
             << endl;
      }
      this_thread::sleep_for(chrono::milliseconds(100));
    }
  };

  thread producerThread(producer);
  thread consumerThread(consumer);

  producerThread.join();
  consumerThread.join();

  assert(queue.isEmpty());
  cout << "Concurrency test passed!" << endl;
}

// Test TaskQueue edge cases
void testTaskQueueEdgeCases() {
  cout << "===== Test TaskQueue Edge Cases =====" << endl;

  TaskQueue noLockQueue(LockType::NoLock, nullptr);
  TaskQueue mutexQueue(LockType::Mutex, nullptr);

  // Test dequeue on empty queue (NoLock)
  Task dequeuedTask;
  assert(!noLockQueue.dequeue(dequeuedTask));
  cout << "NoLock dequeue on empty queue passed!" << endl;

  // Test dequeue on empty queue (Mutex)
  thread producer([&mutexQueue]() {
    this_thread::sleep_for(chrono::milliseconds(100));
    mutexQueue.enqueue(createTask(1, "Task1"));
  });

  assert(mutexQueue.dequeue(dequeuedTask));
  assert(dequeuedTask.id == 1);
  producer.join();
  cout << "Mutex dequeue on empty queue passed!" << endl;

  cout << "All edge cases passed!" << endl;
}

int main() {
  try {
    testTaskQueueBasic();
    testTaskQueueTermination();
    testTaskQueueConcurrency();
    testTaskQueueEdgeCases();
    cout << "\nAll TaskQueue tests passed successfully!" << endl;
  } catch (const exception &e) {
    cerr << "Test failed: " << e.what() << endl;
    return 1;
  }
  return 0;
}

// g++ -std=c++17 -Wall -Wextra -pthread -o testTaskQueue \
//     ../TaskQueue.cpp \
//     ../util/LockImpl/RWLock.cpp \
//     ../util/LockImpl/MutexLock.cpp \
//     testTaskQueue.cpp
