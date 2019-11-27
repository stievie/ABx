#include "stdafx.h"
#include "Crowd.h"
#include "Party.h"
#include <algorithm>

namespace Game {

void Crowd::RegisterLua(kaguya::State& state)
{
    state["Crowd"].setClass(kaguya::UserdataMetatable<Crowd>()
        .addFunction("GetLeader", &Crowd::GetLeader)
        .addFunction("Add", &Crowd::_LuaAdd)
        .addFunction("Remove", &Crowd::_LuaRemove)
    );
}

Crowd::Crowd() :
    id_(Party::GetNewId())
{ }

Crowd::Crowd(uint32_t id) :
    id_(id)
{ }

Npc* Crowd::GetLeader()
{
    if (members_.size() == 0)
        return nullptr;
    if (auto l = members_.front().lock())
        return l.get();
    return nullptr;
}

void Crowd::_LuaRemove(Npc* actor)
{
    if (!actor)
        return;
    Remove(actor->id_);
}

void Crowd::Remove(uint32_t id)
{
    const auto it = std::find_if(members_.begin(), members_.end(), [&](const std::weak_ptr<Npc>& current)
    {
        if (auto c = current.lock())
            return c->id_ == id;
        return false;
    });
    if (it == members_.end())
        return;
    members_.erase(it);
}

void Crowd::_LuaAdd(Npc* actor)
{
    if (!actor)
        return;
    return Add(actor->GetPtr<Npc>());
}

void Crowd::Add(std::shared_ptr<Npc> actor)
{
    const auto it = std::find_if(members_.begin(), members_.end(), [&](const std::weak_ptr<Npc>& current)
    {
        if (auto c = current.lock())
            return c->id_ == actor->id_;
        return false;
    });
    if (it != members_.end())
        return;
    members_.push_back(actor);
    actor->SetGroupId(id_);
}

}
