#pragma once

#include "Task.h"

namespace Asynch {

class Dispatcher
{
public:
    Dispatcher() :
        state_(State::Terminated)
    {
        tasks_.clear();
    }
    ~Dispatcher() {}

    void Start();
    void Stop();
    void Add(Task* task, bool front = false);

    enum State
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
    void DispatcherThread();
public:
    static Dispatcher Instance;
};

}
