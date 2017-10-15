#pragma once

#include <functional>
#include <chrono>
#include <stdint.h>

using the_clock = std::chrono::system_clock;

class Task
{
public:
    Task(unsigned ms, const std::function<void (void)>& f) :
        function_(f),
        expires_(true)
    {
        using namespace std::chrono_literals;
        expiration_ = the_clock::now() + (ms * 1ms);
    }
    Task(const std::function<void(void)>& f) :
        function_(f),
        expiration_(the_clock::time_point::max()),
        expires_(false)
    {}
    ~Task() {}

    /// Execute function
    void operator()()
    {
        function_();
    }

    void SettDontExpires()
    {
        expires_ = false;
    }
    bool IsExpired() const
    {
        if (!expires_)
            // Does not expire
            return false;
        return expiration_ < the_clock::now();
    }
protected:
    the_clock::time_point expiration_;
    bool expires_;
private:
    std::function<void(void)> function_;
};

inline Task* CreateTask(std::function<void(void)> f)
{
    return new Task(f);
}
inline Task* CreateTask(unsigned ms, std::function<void(void)> f)
{
    return new Task(ms, f);
}
