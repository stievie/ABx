#pragma once

#include <map>
#include <memory>
#include <AB/Entities/Effect.h>

namespace Game {

class Effect;

class EffectManager
{
private:
    std::map<uint32_t, AB::Entities::Effect> effects_;
public:
    EffectManager() = default;
    ~EffectManager() = default;

    std::shared_ptr<Effect> Get(uint32_t index);
};

}
