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
    // Min 1 and max 4 threads
    ThreadPool() noexcept :
        numThreads_(std::min<size_t>(4, std::max<size_t>(1, std::thread::hardware_concurrency() - 2))),
        pool_(nullptr)
    { }
    ThreadPool(size_t numThreads) noexcept :
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
        pool_->enqueue(f, args...);
    }
    template<class F, class... Args>
    inline auto EnqueueWithResult(F&& f, Args&&... args)
    {
        if (!pool_)
            throw std::runtime_error("ThreadPool not running");
        return pool_->enqueue(f, args...);
    }

    static ThreadPool Instance;
};

}
