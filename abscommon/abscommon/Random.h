#pragma once

#include <limits>
#include <abcrypto.hpp>
#include <mutex>

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
        std::lock_guard<std::mutex> lock(lock_);
        arc4random_buf(&r, sizeof(T));
        return r;
    }
    void GetBuff(void* buff, size_t len)
    {
        std::lock_guard<std::mutex> lock(lock_);
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
        assert(max > min);
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
