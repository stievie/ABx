#pragma once

#include <limits>
#include <mutex>

namespace Utils {

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
        std::lock_guard<std::mutex> lock(mutex_);
        // When used with unit32_t shouldn't rotate for 136 years if every second is a new ID generated.
        if (id_ >= std::numeric_limits<T>::max())
            id_ = std::numeric_limits<T>::min();
        return ++id_;
    }
    void Reset()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        id_ = std::numeric_limits<T>::min();
    }
private:
    T id_;
};

}
