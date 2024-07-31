#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads) : numThreads(numThreads)
{
    assert(numThreads > 0);
    threads.reserve(numThreads);
    for (size_t i = 0; i < numThreads; ++i) 
    {
        threads.emplace_back(&ThreadPool::Worker, this);
    }
}

void ThreadPool::Enqueue(std::function<void(void*)> f, void* arg) 
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        queue.push_back(Work{ f, arg });
    }
    notEmpty.notify_one();
}

void ThreadPool::Worker() 
{
    while (true) 
    {
        Work w;
        {
            std::unique_lock<std::mutex> lock(mtx);
            notEmpty.wait(lock, [this]() { return !queue.empty() || stop; });
            if (stop && queue.empty()) 
            {
                return;
            }
            w = queue.front();
            queue.pop_front();
        }
        w.f(w.arg);
    }
}

ThreadPool::~ThreadPool() 
{
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
    }
    notEmpty.notify_all();
    for (std::thread& thread : threads) 
    {
        if (thread.joinable()) 
        {
            thread.join();
        }
    }
}