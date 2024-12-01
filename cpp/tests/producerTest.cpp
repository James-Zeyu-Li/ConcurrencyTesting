
#include "../TaskQueue.h"
#include <iostream>
#include <thread>
#include <vector>

void producerTask(TaskQueue &queue, int idStart, int count) {
  for (int i = 0; i < count; ++i) {
    Task task{idStart + i, "Task_" + std::to_string(idStart + i), false};
    queue.enqueue(task);
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 模拟任务间隔
  }
}

void consumerTask(TaskQueue &queue, int count) {
  for (int i = 0; i < count; ++i) {
    Task task;
    if (queue.dequeue(task)) {
      std::cout << "[Consumer] Processed: " << task.id << " - " << task.name
                << std::endl;
    }
  }
}

void testTaskQueue() {
  TaskQueue queue(LockType::Mutex, nullptr); // 使用 Mutex 锁
  const int producerCount = 2;
  const int consumerCount = 1;
  const int tasksPerProducer = 5;

  std::vector<std::thread> producers;
  for (int i = 0; i < producerCount; ++i) {
    producers.emplace_back(producerTask, std::ref(queue), i * tasksPerProducer,
                           tasksPerProducer);
  }

  std::vector<std::thread> consumers;
  for (int i = 0; i < consumerCount; ++i) {
    consumers.emplace_back(consumerTask, std::ref(queue),
                           producerCount * tasksPerProducer);
  }

  for (auto &p : producers) {
    p.join();
  }
  for (auto &c : consumers) {
    c.join();
  }
}

int main() {
  try {
    testTaskQueue();
    std::cout << "TaskQueue threading test passed!" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Test failed: " << e.what() << std::endl;
  }
  return 0;
}

// g++ -std=c++17 -pthread -o producerTest ../TaskQueue.cpp ../util/MutexLock.cpp ../util/RWLock.cpp producerTest.cpp
