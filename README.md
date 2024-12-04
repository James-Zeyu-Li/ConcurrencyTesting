# Concurrency Test for IO concurrency and consumer producer Model

### What is the test

In this project, I aim to have a deeper understanding of concurrency and multithreaded
programming, especially of locks’ influence on performance and behavior. When thinking about
concurrency and especially racing conditions, it’s when at least one thread performs the read,
update, and write cycle that is the critical section to causes the problems. If multiple threads
only read shared data, there's no risk of conflict, and locking is unnecessary. With this in mind, a
new lock mechanism called Read-Write Lock or RWLock looks quite beneficial.


### What's in my design of Producer Consumer CSV

- Queue Design:
    - The queue is not a fixed-length buffer but a dynamic data structure.
    - It stores tasks that are awaiting consumption by consumers.
    - The queue grows dynamically without predefined limits, ensuring flexibility for various test scenarios and workloads.


- Concurrency and Locking Mechanism:
    - The queue utilizes locks with a specified lock type (e.g., MutexLock or RWLock) to facilitate concurrent access by multiple threads.
    - A queueMutex (mutex lock) is implemented to prevent race conditions during task insertion or removal.
    - This ensures safe and efficient access to the shared task queue but slightly reduces concurrency potential due to lock contention.


- Critical Section Protection:

    - Locks are applied to safeguard critical sections where the queue is modified, including enqueue and dequeue operations.
    - This prevents interference between producers and consumers, ensuring:
        - Producers can safely add tasks to the queue.
        - Consumers can retrieve tasks from the queue without inconsistencies or conflicts.


- Efficiency Improvement with Conditional Variables:

    - Conditional variables are used to implement a signal-and-wait mechanism:
        - When the task queue is empty, instead of using a spinning lock or periodic checks (which waste CPU cycles), producers signal consumers once a task is enqueued.
        - This notification mechanism eliminates unnecessary CPU overhead, improving system efficiency during idle or low-load periods.



### Structure of the Files
```
ConcurrencyTesting/
├── BenchmarkManager/
│   ├── BenchmarkTool.h
│   ├── BenchmarkTool.cpp	// Tools and logic
│   ├── RunBenchmark.cpp    // The testing entrence
├── cpp/
│   ├── util/
│   │   ├── ThreadManager.h
│   │   ├── ThreadManager.cpp
│   │   ├── LockType.h
│   │   ├── MutexLock.h
│   │   ├── MutexLock.cpp
│   │   ├── RWLock.h
│   │   └── RWLock.cpp
│   ├── ProducerConsumerConcurrentIO.h 
│   ├── ProducerConsumerConcurrentIO.cpp 
│   ├── TaskQueue.h
│   ├── TaskQueue.cpp
│   ├── CSVHandler.h
│   ├── CSVHandler.cpp
```


### Findings and Results:
It’s interesting to explore multithreaded programming for the first time, is both exciting and also
quite frustrating, since the complexity is growing along with the number of threads. This might
explain why it’s only in the p_
thread library not included in the current std thread library.
1.  When the workload is not extensive, an application of multithread and Locks brings more
overhead than any improvements. Even though

2.  As we can see above, in most cases the RWLock is performing relatively similarly to the
Mutex Lock. In extreme circumstances especially with a lot more reading tasks than writing, the
RWLock performs mutex Lock


3. Performance with 2 Producers and 8 Consumers:

    - For 10,000 tasks, the running times for RWLock and MutexLock are similar.
    - RWLock demonstrates a slightly faster performance in this configuration.

4. Performance with 8 Producers and 2 Consumers:

    - MutexLock performs better than RWLock in this scenario.
    - The performance gap is due to RWLock's larger overhead from lock switching during write operations.

5. General Observations on Total Execution Time:

    - RWLock often completes tasks more quickly than MutexLock.
    - The difference becomes more noticeable in extreme configurations with higher contention levels.

4. Enqueue and Dequeue Timing Distribution:

    - Average enqueue and dequeue times for RWLock are slightly better than MutexLock.
    - However, the differences are minimal at the millisecond level, making them less impactful for large-scale operations.


### Future Work
- The workload I have tested is still pretty lightweight, an interaction with a database or
interaction through a TCP port would be more interesting which provides more
opportunities to discover.
- Doing a comparison between Go, Java, and C++ among the same task would be
interesting to see the difference not only in efficiency but also in implementation difficulty.