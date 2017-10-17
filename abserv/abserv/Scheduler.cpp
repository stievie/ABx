#include "stdafx.h"
#include "Scheduler.h"
#include "Logger.h"
#include "Dispatcher.h"

namespace Asynch {

Scheduler Scheduler::Instance;

void Scheduler::SchedulerThread(void* p)
{
#ifdef DEBUG_SCHEDULER
    LOG_DEBUG << "Scheduler threat started" << std::endl;
#endif

    Scheduler* scheduler = (Scheduler*)p;

    std::unique_lock<std::mutex> lockUnique(scheduler->lock_, std::defer_lock);
    while (scheduler->state_ != State::Terminated)
    {
        ScheduledTask* task = nullptr;
        bool runTask = false;
        bool ret = false;

        lockUnique.lock();

        if (scheduler->events_.empty())
        {
#ifdef DEBUG_SCHEDULER
            LOG_DEBUG << "No events" << std::endl;
#endif
            scheduler->signal_.wait(lockUnique);
        }
        else
        {
#ifdef DEBUG_SCHEDULER
            LOG_DEBUG << "Waiting for event" << std::endl;
#endif
            ret = scheduler->signal_.wait_until(
                lockUnique,
                scheduler->events_.top()->GetCycle()
            ) == std::cv_status::no_timeout;
        }

#ifdef DEBUG_SCHEDULER
        LOG_DEBUG << "Scheduler signaled" << std::endl;
#endif

        if (!ret && (scheduler->state_ != State::Terminated))
        {
            // Timeout
            task = scheduler->events_.top();
            scheduler->events_.pop();

            auto it = scheduler->eventIds_.find(task->GetEventId());
            if (it != scheduler->eventIds_.end())
            {
                // Found so not stopped -> run it
                runTask = true;
                scheduler->eventIds_.erase(it);
            }
        }

        lockUnique.unlock();

        if (task)
        {
            // Add it to the Dispatcher
            if (runTask)
            {
                task->SettDontExpires();
#ifdef DEBUG_SCHEDULER
                LOG_DEBUG << "Executing event " << task->GetEventId() << std::endl;
#endif
                Dispatcher::Instance.Add(task);
            }
            else
                // Was stopped
                delete task;
        }
    }
#ifdef DEBUG_SCHEDULER
    LOG_DEBUG << "Scheduler threat stopped" << std::endl;
#endif
}

uint32_t Scheduler::Add(ScheduledTask* task)
{
    bool doSignal = false;
    lock_.lock();

    if (state_ == State::Running)
    {
        // Check for valid ID
        if (task->GetEventId() == 0)
        {
            // Generate new ID
            if (lastEventId_ >= 0xFFFFFFFF)
                lastEventId_ = 0;
            ++lastEventId_;
            task->SetEventId(lastEventId_);
        }
        // Insert this ID in the list of active events
        eventIds_.insert(task->GetEventId());

        // Add it to the Queue
        events_.push(task);
        // Signal if the list was empty or this event is at the top
        doSignal = (task == events_.top());

#ifdef DEBUG_SCHEDULER
        LOG_DEBUG << "Added event" << task->GetEventId() << std::endl;
#endif
    }
    else
        LOG_ERROR << "Scheduler thread not running" << std::endl;

    lock_.unlock();
    if (doSignal)
        signal_.notify_one();

    return task->GetEventId();
}

bool Scheduler::StopEvent(uint32_t eventId)
{
    if (eventId == 0)
        return false;

#ifdef DEBUG_SCHEDULER
    LOG_DEBUG << "Stopping event " << eventId;
#endif

    lock_.lock();

    auto it = eventIds_.find(eventId);
    if (it != eventIds_.end())
    {
        eventIds_.erase(it);
        lock_.unlock();
        return true;
    }

    // Not found
    lock_.unlock();
    return false;
}

void Scheduler::Start()
{
    state_ = State::Running;
    thread_ = std::thread(std::bind(&Scheduler::SchedulerThread, (void*)this));
}

void Scheduler::Stop()
{
    lock_.lock();
    state_ = State::Closing;
    lock_.unlock();
}

void Scheduler::Terminate()
{
    lock_.lock();
    state_ = State::Terminated;
    while (!events_.empty())
    {
        events_.pop();
    }
    eventIds_.clear();
    lock_.unlock();
}

}
