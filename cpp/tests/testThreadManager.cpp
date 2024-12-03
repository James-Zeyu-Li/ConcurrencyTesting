#include "../CSVHandler.h"
#include "../ProducerConsumerConcurrentIO.h"
#include "../TaskQueue.h"
#include "../util/LockType.h"
#include "../util/MutexLock.h"
#include "../util/RWLock.h"
#include "../util/ThreadManager.h"
#include <cassert>
#include <cstdio> // 添加此头文件以使用remove函数
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

// Test function: Validate customTasks functionality
void testCustomTasks() {
    cout << "\n===== Test: customTasks =====\n" << endl;

    // Prepare test environment
    string testFilePath = "test_output.csv";
    // 在执行customTasks之前，删除现有的CSV文件
    if (remove(testFilePath.c_str()) != 0) {
        perror("Error deleting file");
    } else {
        cout << "Previous CSV file deleted successfully." << endl;
    }

    // Reset global task counter
    ProducerConsumerConcurrentIO::resetGlobalTaskCounter();

    shared_ptr<TaskQueue> taskQueue =
        make_shared<TaskQueue>(LockType::Mutex, nullptr);

    ProducerConsumerConcurrentIO producerConsumerIO(testFilePath, taskQueue);

    // Test parameters
    int producerThreads = 2;     // Number of producer threads
    int produceCount = 20;       // Total tasks to produce
    int readerThreads = 1;       // Number of reader threads
    int consumerThreads = 2;     // Number of consumer threads
    int expectedWriteCount = 20; // Expected rows written to CSV

    // Execute the customTasks function
    producerConsumerIO.customTasks(producerThreads, produceCount, readerThreads,
                                   consumerThreads, expectedWriteCount);

    // Validate task completion count
    cout << "Expected Write Count: " << expectedWriteCount << endl;
    cout << "Actual Tasks Completed: " << producerConsumerIO.getTasksCompleted() << endl;
    assert(producerConsumerIO.getTasksCompleted() == expectedWriteCount);
    cout << "Tasks completed: " << producerConsumerIO.getTasksCompleted() << endl;

    // Validate CSV file content
    auto csvContent = producerConsumerIO.getCSVContent();
    cout << "Expected CSV rows: " << expectedWriteCount << endl;
    cout << "Actual CSV rows: " << csvContent.size() << endl;
    assert(csvContent.size() == static_cast<size_t>(expectedWriteCount));
    cout << "CSV rows written: " << csvContent.size() << endl;

    for (const auto &row : csvContent) {
        assert(row.size() == 3); // Validate CSV row structure: ID, Name, Status
        cout << "Row: ";
        for (const auto &cell : row) {
            cout << cell << " ";
        }
        cout << endl;
    }

    cout << "customTasks test passed successfully!" << endl;
}

// Test function: Edge case with 0 producers
void testCustomTasksZeroProducers() {
    cout << "\n===== Test: customTasks with Zero Producers =====\n" << endl;

    string testFilePath = "test_zero_producers.csv";
    // 在执行customTasks之前，删除现有的CSV文件
    if (remove(testFilePath.c_str()) != 0) {
        perror("Error deleting file");
    } else {
        cout << "Previous CSV file deleted successfully." << endl;
    }

    // Reset global task counter
    ProducerConsumerConcurrentIO::resetGlobalTaskCounter();

    shared_ptr<TaskQueue> taskQueue =
        make_shared<TaskQueue>(LockType::Mutex, nullptr);

    ProducerConsumerConcurrentIO producerConsumerIO(testFilePath, taskQueue);

    int producerThreads = 0;
    int produceCount = 0;
    int readerThreads = 1;
    int consumerThreads = 2;
    int expectedWriteCount = 0;

    // Execute the customTasks function
    producerConsumerIO.customTasks(producerThreads, produceCount, readerThreads,
                                   consumerThreads, expectedWriteCount);

    // Validate task completion count
    cout << "Expected Write Count: " << expectedWriteCount << endl;
    cout << "Actual Tasks Completed: " << producerConsumerIO.getTasksCompleted() << endl;
    assert(producerConsumerIO.getTasksCompleted() == expectedWriteCount);
    cout << "Tasks completed: " << producerConsumerIO.getTasksCompleted() << endl;

    // Validate CSV file content
    auto csvContent = producerConsumerIO.getCSVContent();
    cout << "Expected CSV rows: " << expectedWriteCount << endl;
    cout << "Actual CSV rows: " << csvContent.size() << endl;
    assert(csvContent.empty());
    cout << "CSV rows written: " << csvContent.size() << endl;

    cout << "customTasks with Zero Producers test passed successfully!" << endl;
}

// Test function: Stress test with high task count
void testCustomTasksStress() {
    cout << "\n===== Test: customTasks Stress Test =====\n" << endl;

    string testFilePath = "test_stress.csv";
    // 在执行customTasks之前，删除现有的CSV文件
    if (remove(testFilePath.c_str()) != 0) {
        perror("Error deleting file");
    } else {
        cout << "Previous CSV file deleted successfully." << endl;
    }

    // Reset global task counter
    ProducerConsumerConcurrentIO::resetGlobalTaskCounter();

    shared_ptr<TaskQueue> taskQueue =
        make_shared<TaskQueue>(LockType::Mutex, nullptr);

    ProducerConsumerConcurrentIO producerConsumerIO(testFilePath, taskQueue);

    int producerThreads = 10;
    int produceCount = 1000; // Large number of tasks
    int readerThreads = 3;
    int consumerThreads = 5;
    int expectedWriteCount = 1000;

    // Execute the customTasks function
    producerConsumerIO.customTasks(producerThreads, produceCount, readerThreads,
                                   consumerThreads, expectedWriteCount);

    // Validate task completion count
    cout << "Expected Write Count: " << expectedWriteCount << endl;
    cout << "Actual Tasks Completed: " << producerConsumerIO.getTasksCompleted() << endl;
    assert(producerConsumerIO.getTasksCompleted() == expectedWriteCount);
    cout << "Tasks completed: " << producerConsumerIO.getTasksCompleted() << endl;

    // Validate CSV file content
    auto csvContent = producerConsumerIO.getCSVContent();
    cout << "Expected CSV rows: " << expectedWriteCount << endl;
    cout << "Actual CSV rows: " << csvContent.size() << endl;
    assert(csvContent.size() == static_cast<size_t>(expectedWriteCount));
    cout << "CSV rows written: " << csvContent.size() << endl;

    cout << "customTasks Stress Test passed successfully!" << endl;
}

// Main function to execute all tests
int main() {
    try {
        testCustomTasks();
        testCustomTasksZeroProducers();
        testCustomTasksStress();
        cout << "\nAll tests passed successfully!" << endl;
    } catch (const exception &e) {
        cerr << "Test failed: " << e.what() << endl;
    }

    return 0;
}

//  g++ -std=c++17 -Wall -Wextra -o testProducerConsumerConcurrentIO \
//     ../ProducerConsumerConcurrentIO.cpp \
//     ../CSVHandler.cpp \
//     ../util/ThreadManager.cpp \
//     ../TaskQueue.cpp \
//     ../util/RWLock.cpp \
//     ../util/MutexLock.cpp \
//     testThreadManager.cpp -pthread