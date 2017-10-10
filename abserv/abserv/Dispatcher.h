#pragma once

#include <mutex>
#include <list>
#include <condition_variable>
#include "Task.h"

class Dispatcher
{
public:
    Dispatcher();
    ~Dispatcher();

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
    std::mutex taskLock_;
    std::list<Task*> tasks_;
    State state_;
    std::condition_variable taskSignal_;
    static void DispatcherThread(void* p);
public:
    static Dispatcher Instance;
};

