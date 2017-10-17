#pragma once

#include <stdint.h>
#include "Task.h"
#include <set>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>

#define SCHEDULER_MINTICKS 50

namespace Asynch {

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
    ScheduledTask(uint32_t delay, const std::function<void(void)>& f) :
        Task(delay, f),
        eventId_(0)
    {}

    friend ScheduledTask* CreateScheduledTask(uint32_t delay, const std::function<void(void)>&);
private:
    uint32_t eventId_;
};

inline ScheduledTask* CreateScheduledTask(uint32_t delay, const std::function<void(void)>& f)
{
    assert(delay != 0);
    if (delay < SCHEDULER_MINTICKS)
        delay = SCHEDULER_MINTICKS;
    return new ScheduledTask(delay, f);
}

class LessSchedTask : public std::binary_function<ScheduledTask*&, ScheduledTask*&, bool>
{
public:
    bool operator()(ScheduledTask*& t1, ScheduledTask*& t2)
    {
        return (*t1) < (*t2);
    }
};

class Scheduler
{
public:
    enum State {
        Running,
        Closing,
        Terminated
    };
private:
    State state_;
    uint32_t lastEventId_;
    std::mutex lock_;
    std::condition_variable signal_;
    std::set<uint32_t> eventIds_;
    std::thread thread_;
    std::priority_queue<ScheduledTask*, std::vector<ScheduledTask*>, LessSchedTask> events_;
    static void SchedulerThread(void* p);
public:
    Scheduler() :
        state_(State::Terminated),
        lastEventId_(0)
    {}
    ~Scheduler() {}

    uint32_t Add(ScheduledTask* task);
    bool StopEvent(uint32_t eventId);

    void Start();
    void Stop();
    void Terminate();

    static Scheduler Instance;
};

}
