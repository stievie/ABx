#include "stdafx.h"
#include "Random.h"

#include "DebugNew.h"

namespace Utils {

Random Random::Instance;

void Random::Initialize()
{
    rng::tsc_seed seed;
    gen_ = rng::rng64(seed());
}

bool Random::GetBool()
{
    return Get<uint32_t>(1) == 1;
}

/// 0..1
float Random::GetFloat()
{
    return (float)Get<uint32_t>() / (float)std::numeric_limits<uint32_t>::max();
}

}
