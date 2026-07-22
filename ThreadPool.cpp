//
// Created by nikolaj on 12/29/25.
//

#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([this]() {
            //Keep looping (until the queue is empty and we have been told to stop)
            while (true) {
                std::function<void()> task;
                //Limited scope kills the lock at the end, freeing the mutex for other threads
                {
                    //The lock remains on, except while waiting
                    std::unique_lock<std::mutex> lock(queueGuard);

                    //Waiting with a predicate catches spurious wake-ups
                    condition.wait(lock, [this] {return !taskQueue.empty() || stop || cancelled; });

                    //exit if the pool is to stop AND the queue is empty
                    //That means stop doesn't really mean stop now, it just means stop when you are done
                    if (stop && taskQueue.empty())
                        return;

                    //The cancel flag on the other hand doesn't care about the remaining queue
                    if (cancelled)
                        return;
                    //Move over the next task from the queue
                    task = std::move(taskQueue.front());
                    taskQueue.pop();
                }
                //Execute the task, after freeing the mutex
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    //Limited scope to unlock the mutex when we leave it
    {
        std::unique_lock<std::mutex> lock(queueGuard);
        //Flag that stops the threads from waiting, not a flag to instantly quit
        stop = true;
    }
    //It is important that the notification happens after the scope has quit
    //Otherwise the mutex would prevent the threads from leaving the wait() and we would be stuck waiting for them

    //Free all threads from wait
    condition.notify_all();

    //Wait for all threads to have completed ALL tasks
    for (auto& thread : threads)
        thread.join();
}

void ThreadPool::enqueue(std::function<void()> task) {
    //Limited scope to unlock the mutex when we leave it
    {
        std::unique_lock<std::mutex> lock(queueGuard);
        //Move the task to the queue
        taskQueue.emplace(std::move(task));
    }
    //If there are any threads waiting right now, dispatch them to deal with the task
    condition.notify_one();
}

void ThreadPool::cancel() {
    {
        std::unique_lock<std::mutex> lock(queueGuard);
        stop = true;
        cancelled = true;
        //Clear the queue too, since cancelled means "stop soon"
        while (!taskQueue.empty()) taskQueue.pop();
    }
    condition.notify_all();
}



