#pragma once

#include "GameObject.h"
#include "Effect.h"

namespace Game {

/// Player, NPC, Monster some such
class Creature : public GameObject
{
public:
    EffectList effects_;
};

}
