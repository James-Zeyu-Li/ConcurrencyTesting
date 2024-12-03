package taskQueue

import (
	"sync"
	"sync/atomic"
	"time"
)

// Task represents a unit of work
type Task struct {
	ID        int
	Name      string
	Completed bool
}

// TaskQueue represents a thread-safe queue for tasks
type TaskQueue struct {
	queue chan Task
	mu    sync.RWMutex
	// Benchmark metrics
	totalEnqueueTime int64
	enqueueCount     int64
	maxEnqueueTime   int64
	minEnqueueTime   int64
	totalDequeueTime int64
	dequeueCount     int64
	maxDequeueTime   int64
	minDequeueTime   int64
	blockCount       int64
}

// NewTaskQueue creates a new TaskQueue with a specified buffer size
func NewTaskQueue(bufferSize int) *TaskQueue {
	return &TaskQueue{
		queue:          make(chan Task, bufferSize),
		minEnqueueTime: int64(^uint(0) >> 1), // Initialize to max int
		minDequeueTime: int64(^uint(0) >> 1),
	}
}

// Enqueue adds a task to the queue
func (tq *TaskQueue) Enqueue(task Task) {
	start := time.Now()
	tq.queue <- task
	duration := time.Since(start).Microseconds()

	atomic.AddInt64(&tq.totalEnqueueTime, duration)
	atomic.AddInt64(&tq.enqueueCount, 1)

	for {
		currentMax := atomic.LoadInt64(&tq.maxEnqueueTime)
		if duration > currentMax {
			if atomic.CompareAndSwapInt64(&tq.maxEnqueueTime, currentMax, duration) {
				break
			}
		} else {
			break
		}
	}

	for {
		currentMin := atomic.LoadInt64(&tq.minEnqueueTime)
		if duration < currentMin {
			if atomic.CompareAndSwapInt64(&tq.minEnqueueTime, currentMin, duration) {
				break
			}
		} else {
			break
		}
	}
}

// Dequeue removes a task from the queue
func (tq *TaskQueue) Dequeue() (Task, bool) {
	start := time.Now()
	task, ok := <-tq.queue
	duration := time.Since(start).Microseconds()

	if ok {
		atomic.AddInt64(&tq.totalDequeueTime, duration)
		atomic.AddInt64(&tq.dequeueCount, 1)

		for {
			currentMax := atomic.LoadInt64(&tq.maxDequeueTime)
			if duration > currentMax {
				if atomic.CompareAndSwapInt64(&tq.maxDequeueTime, currentMax, duration) {
					break
				}
			} else {
				break
			}
		}

		for {
			currentMin := atomic.LoadInt64(&tq.minDequeueTime)
			if duration < currentMin {
				if atomic.CompareAndSwapInt64(&tq.minDequeueTime, currentMin, duration) {
					break
				}
			} else {
				break
			}
		}
	}

	return task, ok
}

// IsEmpty checks if the queue is empty
func (tq *TaskQueue) IsEmpty() bool {
	tq.mu.RLock()
	defer tq.mu.RUnlock()
	return len(tq.queue) == 0
}

// QueueSize returns the current size of the queue
func (tq *TaskQueue) QueueSize() int {
	tq.mu.RLock()
	defer tq.mu.RUnlock()
	return len(tq.queue)
}

// Benchmark metrics getters
func (tq *TaskQueue) GetTotalEnqueueTime() int64 { return atomic.LoadInt64(&tq.totalEnqueueTime) }
func (tq *TaskQueue) GetTotalDequeueTime() int64 { return atomic.LoadInt64(&tq.totalDequeueTime) }
func (tq *TaskQueue) GetAverageEnqueueTime() float64 {
	count := atomic.LoadInt64(&tq.enqueueCount)
	if count == 0 {
		return 0
	}
	return float64(tq.totalEnqueueTime) / float64(count)
}
func (tq *TaskQueue) GetAverageDequeueTime() float64 {
	count := atomic.LoadInt64(&tq.dequeueCount)
	if count == 0 {
		return 0
	}
	return float64(tq.totalDequeueTime) / float64(count)
}
func (tq *TaskQueue) GetMaxEnqueueTime() int64 { return atomic.LoadInt64(&tq.maxEnqueueTime) }
func (tq *TaskQueue) GetMinEnqueueTime() int64 { return atomic.LoadInt64(&tq.minEnqueueTime) }
func (tq *TaskQueue) GetMaxDequeueTime() int64 { return atomic.LoadInt64(&tq.maxDequeueTime) }
func (tq *TaskQueue) GetMinDequeueTime() int64 { return atomic.LoadInt64(&tq.minDequeueTime) }
func (tq *TaskQueue) GetBlockCount() int64     { return atomic.LoadInt64(&tq.blockCount) }
