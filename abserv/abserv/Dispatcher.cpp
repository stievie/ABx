#include "stdafx.h"
#include "Dispatcher.h"
#include <thread>

Dispatcher Dispatcher::Instance;

Dispatcher::Dispatcher() :
    state_(State::Terminated)
{
}


Dispatcher::~Dispatcher()
{
}


void Dispatcher::Start()
{
    state_ = State::Running;
    std::thread(DispatcherThread, (void*)this);
}

void Dispatcher::Stop()
{
    taskLock_.lock();
    state_ = State::Closing;
    taskLock_.unlock();
}

void Dispatcher::Terminate()
{
    taskLock_.lock();
    state_ = State::Terminated;
    taskLock_.unlock();
}

void Dispatcher::Add(Task* task, bool front /* = false */)
{
    bool doSignal = false;
    taskLock_.lock();

    if (state_ == State::Running)
    {
        doSignal = tasks_.empty();

        if (!front)
            tasks_.push_back(task);
        else
            tasks_.push_front(task);
    }

    taskLock_.unlock();

    if (doSignal)
        taskSignal_.notify_one();
}

void Dispatcher::DispatcherThread(void* p)
{
    Dispatcher* dispatcher = (Dispatcher*)p;

    // NOTE: second argument defer_lock is to prevent from immediate locking
    std::unique_lock<std::mutex> taskLoockUnique(dispatcher->taskLock_, std::defer_lock);

    while (dispatcher->state_ == State::Running)
    {
        Task* task = nullptr;

        taskLoockUnique.lock();

        if (dispatcher->tasks_.empty())
        {
            // List is empty, wait for signal
            dispatcher->taskSignal_.wait(taskLoockUnique);
        }

        if (!dispatcher->tasks_.empty() && (dispatcher->state_ == State::Running))
        {
            // Take first task
            task = dispatcher->tasks_.front();
            dispatcher->tasks_.pop_front();
        }

        taskLoockUnique.unlock();

        // Execute the task
        if (task)
        {
            (*task)();

            delete task;
        }
    }
}
