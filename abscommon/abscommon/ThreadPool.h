#pragma once

#include <thread_pool.h>
#include <thread>

namespace Asynch {

class ThreadPool
{
private:
    size_t numThreads_;
    std::unique_ptr<thread_pool> pool_;
public:
    ThreadPool() :
        numThreads_(std::thread::hardware_concurrency()),
        pool_(nullptr)
    { }
    ThreadPool(size_t numThreads) :
        numThreads_(numThreads),
        pool_(nullptr)
    { }
    ~ThreadPool();

    void Start();
    void Stop();

    template<class F, class... Args>
    inline void Enqueue(F&& f, Args&&... args)
    {
        if (!pool_)
            return;
        pool_->enqueue(f, std::forward<Args>(args)...);
    }
    template<class F, class... Args>
    inline auto EnqueueWithResult(F&& f, Args&&... args)
    {
        if (!pool_)
            throw std::runtime_error("ThreadPool not running");
        return pool_->enqueue(f, std::forward<Args>(args)...);
    }

    static ThreadPool Instance;
};

}
