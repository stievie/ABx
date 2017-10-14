#pragma once

#include <functional>
#include <chrono>
#include <stdint.h>

class Task
{
public:
    Task(unsigned ms, std::function<void (void)>& f) :
        function_(f)
    {
        expiration_ = std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count() + ms;
    }
    Task(std::function<void(void)>& f) :
        function_(f),
        expiration_(-1)
    {}
    ~Task() {}

    /// Execute function
    void operator()()
    {
        function_();
    }

    bool IsExpired() const
    {
        if (expiration_ < 0)
            // Does not expire
            return false;
        return expiration_ < std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now().time_since_epoch()).count();
    }
private:
    std::function<void(void)> function_;
    int64_t expiration_;
};

inline Task* CreateTask(std::function<void(void)> f)
{
    return new Task(f);
}
inline Task* CreateTask(unsigned ms, std::function<void(void)> f)
{
    return new Task(ms, f);
}
