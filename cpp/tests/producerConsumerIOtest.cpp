#include "../ProducerConsumerConcurrentIO.h"
#include "../TaskQueue.h"
#include "../util/ThreadManager.h"

#include <thread>
using namespace std;

ThreadManager threadManager;

void testProducerConsumerIO() {
  TaskQueue queue(LockType::Mutex, nullptr);
  ProducerConsumerConcurrentIO manager("test.csv", queue);

  // 启动生产者线程
  cout << "[test] Starting producer thread..." << endl;
  manager.startProducerThread(10);

  // 启动消费者线程
  cout << "[test] Starting consumer thread..." << endl;
  manager.startConsumerThread();

  // 等待生产者线程运行
  this_thread::sleep_for(chrono::seconds(2));

  // 检查队列状态
  cout << "[test] Checking queue status before stopping producer..." << endl;
  cout << "Queue size: " << queue.queueSize() << endl;

  // 停止生产者线程
  cout << "[test] Stopping producer thread..." << endl;
  manager.stopProducerThread();

  // 发送终止信号，确保消费者线程能够退出
  cout << "[test] Stopping consumer thread..." << endl;
  manager.stopConsumerThread(1); // 1 表示一个消费者线程

  // 等待消费者线程处理完终止信号
  this_thread::sleep_for(chrono::seconds(1));

  // 等待所有线程退出
  cout << "[test] Joining all threads..." << endl;
  threadManager.joinAllThreads(); // 确保 threadManager 是公共或可访问的

  // 验证队列状态
  cout << "[test] Checking queue status after stopping producer..." << endl;
  cout << "Queue size: " << queue.queueSize() << endl;

  cout << "[test] ProducerConsumerConcurrentIO test completed!" << endl;
}

int main() {
  try {
    testProducerConsumerIO();
  } catch (const std::exception &e) {
    std::cerr << "Test failed: " << e.what() << std::endl;
  }
  return 0;
}

// g++ -std=c++17 -pthread -o testProducerConsumerIO \
//     ../ProducerConsumerConcurrentIO.cpp \
//     ../TaskQueue.cpp \
//     ../CSVHandler.cpp \
//     ../util/MutexLock.cpp \
//     ../util/RWLock.cpp \
//     ../util/ThreadManager.cpp \
//     producerConsumerIOtest.cpp
