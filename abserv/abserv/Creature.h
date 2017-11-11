#pragma once

#include "GameObject.h"
#include "Effect.h"

namespace Game {

/// Player, NPC, Monster some such
class Creature : public GameObject
{
public:
    static void RegisterLua(kaguya::State& state);
    EffectList effects_;

    /// Move speed: 1 = normal speed
    float speed_ = 1.0f;
};

}
