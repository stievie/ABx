#pragma once

#include <mutex>
#include <list>
#include <condition_variable>
#include "Task.h"
#include <thread>

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
    void Terminate();
    void Add(Task* task, bool front = false);

    enum State
    {
        Running,
        Closing,
        Terminated
    };
private:
    std::mutex lock_;
    std::list<Task*> tasks_;
    State state_;
    std::thread thread_;
    std::condition_variable signal_;
    static void DispatcherThread(void* p);
public:
    static Dispatcher Instance;
};

}
