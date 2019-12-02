#pragma once

#include "Npc.h"
#include <kaguya/kaguya.hpp>
#include "Group.h"

namespace Game {

class Crowd : public Group
{
public:
    static void RegisterLua(kaguya::State& state);

    Crowd();
    explicit Crowd(uint32_t id);
};

}
