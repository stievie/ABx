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

#include <limits>
#include <mutex>

namespace sa {

template <typename T>
class IdGenerator
{
private:
    std::mutex mutex_;
public:
    static constexpr T InvalidId = std::numeric_limits<T>::min();
    IdGenerator() noexcept :
        id_(std::numeric_limits<T>::min())
    { }
    ~IdGenerator() = default;
    T Next()
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        // When used with unit32_t shouldn't rotate for 136 years if every second is a new ID generated.
        if (id_ == std::numeric_limits<T>::max())
            id_ = std::numeric_limits<T>::min();
        return ++id_;
    }
    void Reset()
    {
        std::scoped_lock<std::mutex> lock(mutex_);
        id_ = std::numeric_limits<T>::min();
    }
private:
    T id_;
};

}
