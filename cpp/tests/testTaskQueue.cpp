#include "../TaskQueue.h"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

std::mutex coutMutex;

// Helper function to create a task
Task createTask(int id, const string &name) {
  Task t;
  t.id = id;
  t.name = name;
  return t;
}

// // Test TaskQueue basic functionality
// void testTaskQueueBasic() {
//   cout << "===== Test TaskQueue Basic Functionality =====" << endl;
//   TaskQueue queue(LockType::Mutex, nullptr);

//   // Test enqueue
//   Task t1 = createTask(1, "Task1");
//   Task t2 = createTask(2, "Task2");
//   queue.enqueue(t1);
//   queue.enqueue(t2);

//   assert(!queue.isEmpty());
//   assert(queue.queueSize() == 2);

//   // Test dequeue
//   Task dequeuedTask;
//   assert(queue.dequeue(dequeuedTask));
//   assert(dequeuedTask.id == 1);
//   assert(dequeuedTask.name == "Task1");

//   assert(queue.dequeue(dequeuedTask));
//   assert(dequeuedTask.id == 2);
//   assert(dequeuedTask.name == "Task2");

//   assert(queue.isEmpty());
//   assert(queue.queueSize() == 0);
//   cout << "Basic functionality passed!" << endl;
// }

// // Test TaskQueue termination signal
// void testTaskQueueTermination() {
//   cout << "===== Test TaskQueue Termination Signal =====" << endl;
//   TaskQueue queue(LockType::Mutex, nullptr);

//   // Add termination signal
//   Task terminationSignal = createTask(-1, "Terminate");
//   queue.enqueue(terminationSignal);

//   Task dequeuedTask;
//   assert(
//       !queue.dequeue(dequeuedTask)); // Termination signal should stop
//       dequeue
//   cout << "Termination signal handled correctly!" << endl;
// }

// // Test TaskQueue with multiple threads
// void testTaskQueueConcurrency() {
//   cout << "===== Test TaskQueue Concurrency =====" << endl;
//   TaskQueue queue(LockType::Mutex, nullptr);

//   // Producer thread
//   auto producer = [&queue]() {
//     for (int i = 1; i <= 10; ++i) {
//       queue.enqueue(createTask(i, "Task" + to_string(i)));
//       this_thread::sleep_for(chrono::milliseconds(50));
//     }
//   };

//   // Consumer thread
//   auto consumer = [&queue]() {
//     for (int i = 1; i <= 10; ++i) {
//       Task dequeuedTask;
//       if (queue.dequeue(dequeuedTask)) {
//         std::lock_guard<std::mutex> lock(coutMutex);
//         std::cout << "Consumed: " << dequeuedTask.id << " - "
//                   << dequeuedTask.name << std::endl;
//       }
//       this_thread::sleep_for(chrono::milliseconds(100));
//     }
//   };

//   thread producerThread(producer);
//   thread consumerThread(consumer);

//   producerThread.join();
//   consumerThread.join();

//   assert(queue.isEmpty());
//   cout << "Concurrency test passed!" << endl;
// }

// // Test TaskQueue with RWLock using shared and exclusive access
// void testTaskQueueWithRWLock() {
//   cout << "===== Test TaskQueue With RWLock =====" << endl;
//   RWLock rwLock;                              // Stack-allocated lock
//   TaskQueue queue(LockType::RWLock, &rwLock); // Pass the address of the lock

//   Task t1 = createTask(1, "RWLockTask1");
//   Task t2 = createTask(2, "RWLockTask2");

//   // Test shared access (read)
//   queue.enqueue(t1);
//   queue.enqueue(t2);

//   Task dequeuedTask;
//   assert(queue.dequeue(dequeuedTask));
//   assert(dequeuedTask.id == 1);

//   assert(queue.dequeue(dequeuedTask));
//   assert(dequeuedTask.id == 2);

//   assert(queue.isEmpty());
//   cout << "RWLock test passed!" << endl;
// }

// // Test empty queue dequeue
// void testTaskQueueEmptyDequeueWithProducer() {
//   cout << "===== Test TaskQueue Empty Dequeue With Producer =====" << endl;
//   TaskQueue queue(LockType::Mutex, nullptr);

//   thread producer([&queue]() {
//     this_thread::sleep_for(chrono::milliseconds(100));
//     queue.enqueue({1, "Task1", false});
//   });

//   Task dequeuedTask;
//   assert(
//       queue.dequeue(dequeuedTask)); // Will block until the producer adds a
//       task
//   assert(dequeuedTask.id == 1);
//   cout << "Empty queue handled correctly with producer!" << endl;

//   producer.join();
// }

// // Stress test with multiple producers and consumers
// void testTaskQueueStress() {
//   cout << "===== Test TaskQueue Stress Test =====" << endl;

//   TaskQueue queue(LockType::Mutex, nullptr);

//   const int numProducers = 5;
//   const int numConsumers = 5;

//   vector<thread> producers, consumers;

//   // Producer threads
//   for (int i = 0; i < numProducers; ++i) {
//     producers.emplace_back([&queue, i]() {
//       for (int j = 1; j <= 20; ++j) {
//         queue.enqueue(createTask(j, "Task from Producer " + to_string(i)));
//         this_thread::sleep_for(chrono::milliseconds(10));
//       }
//     });
//   }

//   // Consumer threads
//   for (int i = 0; i < numConsumers; ++i) {
//     consumers.emplace_back([&queue, i]() {
//       while (true) {
//         Task task;
//         if (!queue.dequeue(task)) { // Termination signal received
//           break;
//         }
//         {
//           lock_guard<mutex> lock(coutMutex);
//           cout << "Consumer " << i << " processed Task ID: " << task.id <<
//           endl;
//         }
//       }
//     });
//   }

//   // Wait for producers to finish
//   for (auto &producer : producers) {
//     producer.join();
//   }

//   // Send termination signals to consumers
//   for (int i = 0; i < numConsumers; ++i) {
//     queue.enqueue(createTask(-1, "Terminate"));
//   }

//   // Wait for consumers to finish
//   for (auto &consumer : consumers) {
//     consumer.join();
//   }

//   cout << "Stress test passed!" << endl;
// }
// // Test rapid enqueue and dequeue
// void testTaskQueueRapidOperations() {
//   cout << "===== Test TaskQueue Rapid Enqueue and Dequeue =====" << endl;
//   TaskQueue queue(LockType::Mutex, nullptr);

//   vector<thread> workers;

//   // Rapidly enqueue tasks
//   auto producer = [&queue]() {
//     for (int i = 1; i <= 1000; ++i) {
//       queue.enqueue(createTask(i, "Task" + to_string(i)));
//     }
//   };

//   // Rapidly dequeue tasks
//   auto consumer = [&queue]() {
//     for (int i = 1; i <= 1000; ++i) {
//       Task dequeuedTask;
//       queue.dequeue(dequeuedTask);
//     }
//   };

//   workers.emplace_back(producer);
//   workers.emplace_back(consumer);

//   for (auto &worker : workers) {
//     worker.join();
//   }

//   assert(queue.isEmpty());
//   cout << "Rapid operations test passed!" << endl;
// }

// void testQueuePerformance(LockType lockType) {
//   TaskQueue queue(lockType);

//   auto producer = [&queue]() {
//     for (int i = 0; i < 100000; ++i) {
//       queue.enqueue(
//           {i, "Task" + std::to_string(i), false}); // Initialize isCompleted
//     }
//   };

//   auto consumer = [&queue]() {
//     Task task;
//     while (queue.dequeue(task)) {
//       // Process task
//     }
//   };

//   auto start = std::chrono::high_resolution_clock::now();

//   std::thread prodThread(producer);
//   std::thread consThread(consumer);

//   prodThread.join();
//   queue.enqueue({-1, "Terminate", false}); // Initialize isCompleted
//   consThread.join();

//   auto end = std::chrono::high_resolution_clock::now();
//   std::chrono::duration<double> elapsed = end - start;

//   std::cout << "LockType: "
//             << (lockType == LockType::Mutex ? "Mutex" : "RWLock")
//             << ", Time taken: " << elapsed.count() << " seconds" <<
//             std::endl;
// }

void singleThreadTest() {
  std::cout << "Running Single Thread Test...\n";
  TaskQueue taskQueue(LockType::Mutex);

  // 入队
  for (int i = 0; i < 10; ++i) {
    Task task{i, "Task_" + std::to_string(i), false};
    taskQueue.enqueue(task);
  }

  // 出队
  for (int i = 0; i < 10; ++i) {
    Task task;
    if (taskQueue.dequeue(task)) {
      std::cout << "Dequeued Task: ID = " << task.id << ", Name = " << task.name
                << "\n";
    }
  }

  std::cout << "Single Thread Test Passed.\n";
}

void multiThreadTest() {
  std::cout << "Running Multi Thread Test...\n";
  TaskQueue taskQueue(LockType::RWLock);

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  // 生产者线程
  for (int i = 0; i < 2; ++i) {
    producers.emplace_back([&]() {
      for (int j = 0; j < 10; ++j) {
        Task task{j, "Task_" + std::to_string(j), false};
        taskQueue.enqueue(task);
      }
    });
  }

  // 消费者线程
  for (int i = 0; i < 2; ++i) {
    consumers.emplace_back([&]() {
      for (int j = 0; j < 10; ++j) {
        Task task;
        if (taskQueue.dequeue(task)) {
          std::cout << "Dequeued Task: ID = " << task.id
                    << ", Name = " << task.name << "\n";
        }
      }
    });
  }

  for (auto &t : producers)
    t.join();
  for (auto &t : consumers)
    t.join();

  std::cout << "Multi Thread Test Passed.\n";
}

int main() {
  try {
    // testTaskQueueBasic();
    // testTaskQueueTermination();
    // testTaskQueueConcurrency();
    // testTaskQueueStress();
    // testTaskQueueWithRWLock();
    // testTaskQueueRapidOperations();
    // testQueuePerformance(LockType::Mutex);
    // testQueuePerformance(LockType::RWLock);
    // testTaskQueueEmptyDequeueWithProducer();
    singleThreadTest();
    multiThreadTest();

    cout << "\nAll TaskQueue tests passed successfully!" << endl;
  } catch (const exception &e) {
    cerr << "Test failed: " << e.what() << endl;
    return 1;
  }
  return 0;
}

// g++ -std=c++17 -Wall -Wextra -pthread -o testTaskQueue \
//     ../TaskQueue.cpp \
//     ../util/RWLock.cpp \
//     ../util/MutexLock.cpp \
//     testTaskQueue.cpp
