/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <chrono>
#include <functional>

namespace Asynch {

using the_clock = std::chrono::system_clock;

class Task
{
public:
    explicit Task(unsigned ms, std::function<void(void)>&& f) :
        expires_(true),
        function_(std::move(f))
    {
        expiration_ = the_clock::now() + std::chrono::milliseconds(ms);
    }
    explicit Task(std::function<void(void)>&& f) :
        expiration_(the_clock::time_point::max()),
        expires_(false),
        function_(std::move(f))
    {}
    ~Task() = default;

    /// Execute function
    void operator()()
    {
        function_();
    }

    void SetDontExpires()
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

/// Creates a task that does not expire.
inline Task* CreateTask(std::function<void(void)>&& f)
{
    return new Task(std::move(f));
}
/// Creates a new task that may expire.
/// \param ms Expiration
/// \param f Function
inline Task* CreateTask(unsigned ms, std::function<void(void)>&& f)
{
    return new Task(ms, std::move(f));
}

}
