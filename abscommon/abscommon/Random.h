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
#include <abcrypto.hpp>
#include <mutex>
#include <sa/Assert.h>

namespace Crypto {

class Random
{
private:
    std::mutex lock_;
public:
    Random() = default;
    ~Random() = default;

    void Initialize();
    bool GetBool();
    /// 0..1
    float GetFloat();

    template <typename T>
    T Get()
    {
        T r;
        // Random pool must be locked
        std::scoped_lock lock(lock_);
        arc4random_buf(&r, sizeof(T));
        return r;
    }
    void GetBuff(void* buff, size_t len)
    {
        std::scoped_lock lock(lock_);
        arc4random_buf(buff, len);
    }
    /// Get value from 0..max
    template <typename T>
    T Get(T max)
    {
        return static_cast<T>(GetFloat() * static_cast<float>(max));
    }
    /// Get value from min..max
    template <typename T>
    T Get(T min, T max)
    {
        ASSERT(max > min);
        return static_cast<T>(GetFloat() * static_cast<float>(max - min)) + min;
    }
    /// Test if `test` is smaller or equal to `value` (value = constant value), with max. +/-
    /// random deviation in `percent` of value.
    template <typename T>
    bool Matches(T value, T test, T percent)
    {
        T deviatiton = (value / static_cast<T>(100)) * percent;
        T rnd = Get<T>(value - deviatiton, value + deviatiton);
        return test <= rnd;
    }
};

}
