#include "stdafx.h"
#include "Dispatcher.h"
#include "Logger.h"
#include "OutputMessage.h"

#include "DebugNew.h"

namespace Asynch {

Dispatcher Dispatcher::Instance;

void Dispatcher::Start()
{
    state_ = State::Running;
    thread_ = std::thread(&Dispatcher::DispatcherThread, this);
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
#ifdef DEBUG_DISPATTCHER
    LOG_DEBUG << "Dispatcher threat started" << std::endl;
#endif

    Net::OutputMessagePool* outputPool;

    // NOTE: second argument defer_lock is to prevent from immediate locking
    std::unique_lock<std::mutex> taskLockUnique(lock_, std::defer_lock);

    while (state_ != State::Terminated)
    {
        Task* task = nullptr;

        taskLockUnique.lock();

        if (tasks_.empty())
        {
            // List is empty, wait for signal
            signal_.wait(taskLockUnique);
#ifdef DEBUG_DISPATTCHER
            LOG_DEBUG << "Waiting for task" << std::endl;
#endif
        }

#ifdef DEBUG_DISPATTCHER
        LOG_DEBUG << "Dispatcher signaled" << std::endl;
#endif

        if (!tasks_.empty() && (state_ != State::Terminated))
        {
            // Take first task
            task = tasks_.front();
            tasks_.pop_front();
        }

        taskLockUnique.unlock();

        // Execute the task
        if (task)
        {
            if (!task->IsExpired())
            {
                Net::OutputMessagePool::Instance()->StartExecutionFrame();
                (*task)();

                outputPool = Net::OutputMessagePool::Instance();
                if (outputPool)
                    outputPool->SendAll();
            }

            delete task;

#ifdef DEBUG_DISPATTCHER
            LOG_DEBUG << "Executing task" << std::endl;
#endif
        }
    }
#ifdef DEBUG_DISPATTCHER
    LOG_DEBUG << "Dispatcher threat stopped" << std::endl;
#endif
}

}
