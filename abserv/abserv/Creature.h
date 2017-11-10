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
};

}
