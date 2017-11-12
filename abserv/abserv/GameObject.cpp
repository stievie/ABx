#include "stdafx.h"
#include "GameObject.h"

#include "DebugNew.h"

namespace Game {

uint32_t GameObject::objectIds_ = 0;

void GameObject::RegisterLua(kaguya::State& state)
{
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        .addProperty("Id", &GameObject::GetId)
        .addProperty("Game", &GameObject::GetGame)
    );
}

GameObject::GameObject()
{
    id_ = GetNewId();
}

}
