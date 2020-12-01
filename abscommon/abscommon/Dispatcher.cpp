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

#include "Dispatcher.h"
#include "Utils.h"
#include <sa/time.h>
#include "Logger.h"

//#define DEBUG_DISPATCHER

namespace Asynch {

void Dispatcher::Start()
{
    if (state_ != State::Running)
    {
        state_ = State::Running;
        thread_ = std::thread(&Dispatcher::DispatcherThread, this);
    }
}

void Dispatcher::Stop()
{
    if (state_ == State::Running)
    {
        lock_.lock();
        state_ = State::Terminated;
        while (!tasks_.empty())
        {
            Task* task = tasks_.front();
            tasks_.pop_front();
            delete task;
        }
        lock_.unlock();
        // Notify thread to exit
        signal_.notify_one();
        thread_.join();
    }
}

void Dispatcher::Add(Task* task, bool front /* = false */)
{
    bool doSignal = false;
    lock_.lock();

    if (state_ == State::Running)
    {
        doSignal = tasks_.empty();

        if (!front)
            tasks_.push_back(task);
        else
            tasks_.push_front(task);
    }
    else
        delete task;

    lock_.unlock();

    if (doSignal)
        signal_.notify_one();
}

void Dispatcher::DispatcherThread()
{
#ifdef DEBUG_DISPATCHER
    LOG_DEBUG << "Dispatcher threat started" << std::endl;
#endif

    // NOTE: second argument defer_lock is to prevent from immediate locking
    std::unique_lock<std::mutex> taskLockUnique(lock_, std::defer_lock);

    while (state_ != State::Terminated)
    {
        taskLockUnique.lock();

        const int64_t observationStart = sa::time::tick();

        if (tasks_.empty())
        {
            // List is empty, wait for signal
            signal_.wait(taskLockUnique);
#ifdef DEBUG_DISPATCHER
            LOG_DEBUG << "Waiting for task" << std::endl;
#endif
        }

#ifdef DEBUG_DISPATCHER
        LOG_DEBUG << "Dispatcher signaled" << std::endl;
#endif

        if (!tasks_.empty())
        {
            // Take first task
            Task* task = tasks_.front();
            tasks_.pop_front();
            taskLockUnique.unlock();

            // Execute the task
            if (!task->IsExpired())
            {
                const int64_t startExecTime = sa::time::tick();

                (*task)();

                // https://technet.microsoft.com/en-us/library/cc181325.aspx
                const uint32_t busyTime = sa::time::time_elapsed(startExecTime);
                const uint32_t observationTime = sa::time::time_elapsed(observationStart);
                if (observationTime != 0)
                    utilization_ = static_cast<uint32_t>(busyTime / observationTime);
            }

            delete task;

#ifdef DEBUG_DISPATCHER
            LOG_DEBUG << "Executing task" << std::endl;
#endif
        }
        else
            taskLockUnique.unlock();
    }
#ifdef DEBUG_DISPATCHER
    LOG_DEBUG << "Dispatcher threat stopped" << std::endl;
#endif
}

}
