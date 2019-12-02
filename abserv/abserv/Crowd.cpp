#include "stdafx.h"
#include "Crowd.h"
#include <algorithm>

namespace Game {

void Crowd::RegisterLua(kaguya::State& state)
{
    state["Crowd"].setClass(kaguya::UserdataMetatable<Crowd, Group>()
    );
}

Crowd::Crowd() :
    Group(Group::GetNewId())
{ }

Crowd::Crowd(uint32_t id) :
    Group(id)
{ }

}
