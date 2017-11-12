#pragma once

#include "EffectManager.h"

namespace IO {

class IOEffects
{
public:
    IOEffects() = delete;
    static bool Load(Game::EffectManager& manager, const std::string fileName);
};

}
