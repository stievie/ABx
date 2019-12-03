#pragma once

#include "Task.h"
#include <set>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <sa/IdGenerator.h>

namespace Asynch {

static constexpr unsigned SCHEDULER_MINTICKS = 10u;

class ScheduledTask : public Task
{
public:
    ~ScheduledTask() {}
    void SetEventId(uint32_t eventId) { eventId_ = eventId; }
    uint32_t GetEventId() const { return eventId_; }
    the_clock::time_point GetCycle() const { return expiration_; }
    bool operator < (const ScheduledTask& rhs) const
    {
        return GetCycle() > rhs.GetCycle();
    }
protected:
    ScheduledTask(uint32_t delay, std::function<void(void)>&& f) :
        Task(delay, std::move(f)),
        eventId_(0)
    {}

    friend ScheduledTask* CreateScheduledTask(uint32_t delay, std::function<void(void)>&&);
    friend ScheduledTask* CreateScheduledTask(std::function<void(void)>&&);
private:
    uint32_t eventId_;
};

inline ScheduledTask* CreateScheduledTask(std::function<void(void)>&& f)
{
    return new ScheduledTask(SCHEDULER_MINTICKS, std::move(f));
}

inline ScheduledTask* CreateScheduledTask(uint32_t delay, std::function<void(void)>&& f)
{
    if (delay < SCHEDULER_MINTICKS)
        delay = SCHEDULER_MINTICKS;
    return new ScheduledTask(delay, std::move(f));
}

struct TaskComparator
{
    bool operator()(const ScheduledTask* lhs, const ScheduledTask* rhs) const
    {
        return lhs->GetCycle() > rhs->GetCycle();
    }
};

class Scheduler
{
public:
    enum class State {
        Running,
        Terminated
    };
private:
    State state_;
    std::mutex lock_;
    std::condition_variable signal_;
    std::set<uint32_t> eventIds_;
    std::thread thread_;
    std::priority_queue<ScheduledTask*, std::deque<ScheduledTask*>, TaskComparator> events_;
    sa::IdGenerator<uint32_t> idGenerator_;
    void SchedulerThread();
public:
    Scheduler() :
        state_(State::Terminated)
    {}
    ~Scheduler() = default;

    /// Add a Task, return EventID
    uint32_t Add(ScheduledTask* task);
    bool StopEvent(uint32_t eventId);

    void Start();
    void Stop();
};

}
