#pragma once

class Scheduler
{
public:
    Scheduler();
    ~Scheduler();

    static Scheduler Instance;
    void Start();
};

