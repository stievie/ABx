/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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

    size_t GetNumThreads() const { return numThreads_; }

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
};

}
