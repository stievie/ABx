#pragma once

#include <mutex>
#include <limits>
#include <abcrypto.hpp>

namespace Client {
namespace Utils {

class Random
{
private:
    std::mutex lock_;
public:
    Random() = default;
    ~Random() {}

    void Initialize();
    bool GetBool();
    /// 0..1
    float GetFloat();

    template <typename T>
    T Get()
    {
        T r;
        std::lock_guard<std::mutex> lock(lock_);
        arc4random_buf(&r, sizeof(T));
        return static_cast<T>(r);
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
        return static_cast<T>(static_cast<float>(Get<T>()) /
            static_cast<float>(std::numeric_limits<T>::max()) *
            static_cast<float>(max + 1));
    }
    /// Get value from min..max
    template <typename T>
    T Get(T min, T max)
    {
        assert(max > min);
        return Get<T>(max - min) + min;
    }

    static Random Instance;
};

}
}
