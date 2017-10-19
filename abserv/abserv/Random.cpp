#include "stdafx.h"
#include "Random.h"
#include <limits>

namespace Utils {

Random Random::Instance;

void Random::Initialize()
{
    rng::tsc_seed seed;
    gen_ = rng::rng64(seed());
}

uint64_t Random::GetU64()
{
    std::lock_guard<std::mutex> lock(lock_);
    return gen_();
}

uint32_t Random::GetU32()
{
    return static_cast<uint32_t>(GetU64() >> 16);
}

/// Get value from 0..max
uint32_t Random::GetU32(uint32_t max)
{
    return static_cast<uint32_t>((float)GetU32() / (float)std::numeric_limits<uint32_t>::max() * (float)(max + 1));
}
bool Random::GetBool()
{
    return GetU32(1) == 1;
}

/// 0..1
float Random::GetFloat()
{
    return (float)GetU32() / (float)std::numeric_limits<uint32_t>::max();
}

}
