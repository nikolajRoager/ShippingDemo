//
// Created by nikolaj on 12/29/25.
// Heavily inspired by Geeks for Geeks:  https://www.geeksforgeeks.org/cpp/thread-pool-in-cpp/
// The Code is, however, my own, and has functionalities not found on geeks for geeks,
// among other things I have added more functionalities (such as stop on exception)
//

#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
public:
    ///Create a  threadpool, and start waiting
    explicit ThreadPool(size_t num_threads);
    ///destructor, waits for all current tasks to finish peacefully, does NOT clear the queue
    ~ThreadPool();
    ///Add a task to the queue
    void enqueue(std::function<void()> task);
    ///Forces threadpool to quit ASAP (Does not halt ongoing threads, but clears the queue)
    void cancel();

private:
    ///Flag to peacefully stop when all threads are done (Does not clear the queue, does not halt tasks)
    bool stop = false;
    ///Flag to ignore waiting tasks
    bool cancelled = false;

    std::mutex queueGuard;
    std::condition_variable condition;
    std::vector<std::thread> threads;
    std::queue<std::function<void()> > taskQueue;
};


#endif //THREADPOOL_THREADPOOL_H