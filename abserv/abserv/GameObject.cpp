#include "stdafx.h"
#include "GameObject.h"

#include "DebugNew.h"

namespace Game {

uint32_t GameObject::objectIds_ = 0;

void GameObject::RegisterLua(kaguya::State& state)
{
    state["GameObject"].setClass(kaguya::UserdataMetatable<GameObject>()
        /*        .addFunction("GetName", &Skill::GetName)
        .addFunction("SetName", &Skill::SetName)
        .addFunction("GetDescription", &Skill::GetDescription)
        .addFunction("SetDescription", &Skill::SetDescription)
        .addFunction("GetCooldownTime", &Skill::GetCooldownTime)
        .addFunction("SetCooldownTime", &Skill::SetCooldownTime)*/
    );
}

GameObject::GameObject()
{
    id_ = GetNewId();
}


GameObject::~GameObject()
{
}

}
