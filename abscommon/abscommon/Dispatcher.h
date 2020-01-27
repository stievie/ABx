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

#include "Task.h"
#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace Asynch {

class Dispatcher
{
public:
    Dispatcher() :
        state_(State::Terminated),
        utilization_(0)
    {
        tasks_.clear();
    }
    ~Dispatcher() = default;

    void Start();
    void Stop();
    void Add(Task* task, bool front = false);
    /// CPU Utilization in % something between 0..100
    uint32_t GetUtilization() const
    {
        return utilization_;
    }

    bool IsDispatcherThread() const { return (state_ == State::Running) ? thread_.get_id() == std::this_thread::get_id() : false; }

    enum class State
    {
        Running,
        Terminated
    };
private:
    std::mutex lock_;
    std::list<Task*> tasks_;
    State state_;
    std::thread thread_;
    std::condition_variable signal_;
    uint32_t utilization_;
    void DispatcherThread();
};

}
