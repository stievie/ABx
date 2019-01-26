#include "stdafx.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include "Logger.h"
#include "Subsystems.h"

namespace Asynch {

void Scheduler::SchedulerThread()
{
#ifdef DEBUG_SCHEDULER
    LOG_DEBUG << "Scheduler threat started" << std::endl;
#endif

    std::unique_lock<std::mutex> lockUnique(lock_, std::defer_lock);
    while (state_ != State::Terminated)
    {
        ScheduledTask* task = nullptr;
        std::cv_status ret = std::cv_status::no_timeout;

        lockUnique.lock();

        if (events_.empty())
        {
#ifdef DEBUG_SCHEDULER
//            LOG_DEBUG << "No events" << std::endl;
#endif
            signal_.wait(lockUnique);
        }
        else
        {
#ifdef DEBUG_SCHEDULER
//            LOG_DEBUG << "Waiting for event" << std::endl;
            int64_t waitStart = Utils::AbTick();
#endif
            ret = signal_.wait_until(
                lockUnique,
                events_.top()->GetCycle()
            );
#ifdef DEBUG_SCHEDULER
//            LOG_DEBUG << "Waited " << (Utils::AbTick() - waitStart) << std::endl;
#endif
        }

#ifdef DEBUG_SCHEDULER
//        LOG_DEBUG << "Scheduler signaled" << std::endl;
#endif

        if (ret == std::cv_status::timeout)
        {
#ifdef DEBUG_SCHEDULER
//            LOG_DEBUG << "Timeout" << std::endl;
#endif
            // Timeout
            task = events_.top();
            events_.pop();

            auto it = eventIds_.find(task->GetEventId());
            if (it == eventIds_.end())
            {
                lockUnique.unlock();
                delete task;
                continue;
            }

            eventIds_.erase(it);
            lockUnique.unlock();

            task->SetDontExpires();
            auto disp = GetSubsystem<Asynch::Dispatcher>();
            if (disp)
                disp->Add(task, true);
        }
        else
            lockUnique.unlock();

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
            // Generate new ID
            task->SetEventId(idGenerator_.Next());
        // Insert this ID in the list of active events
        eventIds_.insert(task->GetEventId());

        // Add it to the Queue
        events_.push(task);
        // Signal if the list was empty or this event is at the top
        doSignal = (task == events_.top());

#ifdef DEBUG_SCHEDULER
        LOG_DEBUG << "Added event " << task->GetEventId() << std::endl;
#endif
    }
    else
    {
        LOG_ERROR << "Scheduler thread not running" << std::endl;
        lock_.unlock();
        delete task;
        return 0;
    }

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
    if (state_ != State::Running)
    {
        state_ = State::Running;
        thread_ = std::thread(&Scheduler::SchedulerThread, this);
    }
}

void Scheduler::Stop()
{
    if (state_ == State::Running)
    {
        lock_.lock();
        state_ = State::Terminated;
        while (!events_.empty())
        {
            ScheduledTask* task = events_.top();
            events_.pop();
            delete task;
        }
        eventIds_.clear();
        lock_.unlock();
        signal_.notify_one();
        thread_.join();
    }
}

}
