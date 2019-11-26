#pragma once

#include <vector>
#include <memory>
#include "Npc.h"
#include <kaguya/kaguya.hpp>

namespace Game {

class Crowd
{
private:
    std::vector<std::weak_ptr<Npc>> members_;
    void _LuaAdd(Npc* actor);
public:
    static void RegisterLua(kaguya::State& state);

    Crowd();
    explicit Crowd(uint32_t id);

    Npc* GetLeader();
    void Add(std::shared_ptr<Npc> actor);

    uint32_t id_;
};

}
