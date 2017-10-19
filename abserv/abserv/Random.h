#pragma once

#include <rng.h>
#include <mutex>

namespace Utils {

class Random
{
private:
    rng::rng64 gen_;
    std::mutex lock_;
public:
    Random() = default;
    ~Random() {}

    void Initialize();
    uint64_t GetU64();
    uint32_t GetU32();
    /// Get value from 0..max
    uint32_t GetU32(uint32_t max);
    bool GetBool();
    /// 0..1
    float GetFloat();

    static Random Instance;
};

}
