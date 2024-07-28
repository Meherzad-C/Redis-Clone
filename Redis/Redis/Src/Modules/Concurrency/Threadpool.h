#pragma once

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <cassert>

class ThreadPool 
{
public:
    struct Work
    {
        std::function<void(void*)> f;
        void* arg;
    };

private:
    std::vector<std::thread> threads;
    std::deque<Work> queue;
    std::mutex mtx;
    std::condition_variable notEmpty;
    bool stop = false;

private:
    void Worker();

public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    void Enqueue(std::function<void(void*)> f, void* arg);
};
