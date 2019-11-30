#include "stdafx.h"
#include "Crowd.h"
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
    Group(Group::GetNewId())
{ }

Crowd::Crowd(uint32_t id) :
    Group(id)
{ }

Npc* Crowd::GetLeader()
{
    if (members_.size() == 0)
        return nullptr;
    if (auto l = members_.front().lock())
    {
        if (Is<Npc>(*l))
            return To<Npc>(l.get());
    }
    return nullptr;
}

void Crowd::_LuaRemove(Npc* actor)
{
    if (!actor)
        return;
    Remove(actor->id_);
}

void Crowd::_LuaAdd(Npc* actor)
{
    if (!actor)
        return;
    return Add(actor->GetPtr<Npc>());
}

}
