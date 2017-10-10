#pragma once

#include <functional>

class Task
{
public:
    Task(unsigned ms, std::function<void (void)>& f) :
        function_(f)
    {}
    Task(std::function<void(void)>& f) :
        function_(f)
    {}
    ~Task() {}

    void operator()()
    {
        function_();
    }
private:
    std::function<void(void)> function_;
};

