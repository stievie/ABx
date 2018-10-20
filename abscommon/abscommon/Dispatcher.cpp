#include "stdafx.h"
#include "Dispatcher.h"
#include "Utils.h"

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

        const int64_t observationStart = Utils::AbTick();

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
                const int64_t startExecTime = Utils::AbTick();

                (*task)();

                // https://technet.microsoft.com/en-us/library/cc181325.aspx
                const int64_t busyTime = Utils::AbTick() - startExecTime;
                const int64_t observationTime = Utils::AbTick() - observationStart;
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
