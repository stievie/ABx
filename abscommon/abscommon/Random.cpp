#include "stdafx.h"
#include "Random.h"

namespace Crypto {

void Random::Initialize()
{
    arc4random_stir();
}

bool Random::GetBool()
{
    return Get<uint32_t>(1) == 1;
}

/// 0..1
float Random::GetFloat()
{
    return static_cast<float>(Get<uint32_t>()) / static_cast<float>(std::numeric_limits<uint32_t>::max());
}

}
