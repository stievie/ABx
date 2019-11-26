#pragma once

#include "Npc.h"
#include <kaguya/kaguya.hpp>
#include <memory>
#include <vector>

namespace Game {

class Crowd
{
private:
    std::vector<std::weak_ptr<Npc>> members_;
    void _LuaAdd(Npc* actor);
    void _LuaRemove(Npc* actor);
public:
    static void RegisterLua(kaguya::State& state);

    Crowd();
    explicit Crowd(uint32_t id);

    Npc* GetLeader();
    void Add(std::shared_ptr<Npc> actor);
    void Remove(uint32_t id);

    uint32_t id_;
};

}
