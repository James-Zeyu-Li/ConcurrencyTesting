# Concurrency Test for IO concurrency and consumer producer Model


### Concurrency testing plan

1. Test the total running time between each type of action with the same amount of Tasks
    - compare single thread, multi-thread, MutexLock, RWLock
2. test thread throughput per thread per second
3. Test each thread throughput count and active time
4. Test thread activity through different types of locks, block times of actions, waiting and active rate
5. Lock being switched from one end to another count
6. for a consumer model, queue length in a different circumstance
    - consumer > producer
    - producer > consumer
    - balanced
7. producer and consumer blocking count
    - MutexLock and RWLock
8: Test Context Switching frequency between different locks
9. Check if the data is complete.

- Might happen: test rapid thread context switch, spinning lock
- Might: Add a lock free circumstance by using Atomic instead of lock


### Implementation Step-by-Step Plan

1. implement MultithreadConcurrentIO
- Single thread read only, single thread write only
- one thread read and one thread write at the same time
- read thread count > write thread count
- write thread count > read thread count
- Multi-thread read and Multithread write at the same time 


2. implement ProducerConsumerModel
- Create a priority queue of jobs
- Single thread read and write at the same time
- read thread count > write thread count
- write thread count > read thread count
- Multi-thread read and Multithread write at the same time 


3. Testing framework
    1. test if the threads are working properly
    2. Execution count per time length
    3. Thread count and resource utilization.

4. MultithreadConcurrentIO

- Add a lock to multithread read and write 
- Add begin immediately, begin exclusive, begin deferred simulation
- Add random position read and write (if time allows)


5. ProducerConsumerModleTest

- multi-thread read write Mutax lock
- multithread read write RWlock
- mixed lock testing 
- deadlock situation simulation ( if time allows)

6. Testing Framework

- benchmark the time and efficiency of multithread

7. First apply the testing simulation to the file IO
	- If time allows later implement Database IO


