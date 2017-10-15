#pragma once

#define SCHEDULER_MINTICKS 50

class Scheduler
{
public:
    Scheduler();
    ~Scheduler();

    static Scheduler Instance;
    void Start();
};

