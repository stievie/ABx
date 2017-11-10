#include "stdafx.h"
#include "Creature.h"

namespace Game {

void Creature::RegisterLua(kaguya::State& state)
{
    GameObject::RegisterLua(state);
    state["Creature"].setClass(kaguya::UserdataMetatable<Creature, GameObject>()
        /*        .addFunction("GetName", &Skill::GetName)
        .addFunction("SetName", &Skill::SetName)
        .addFunction("GetDescription", &Skill::GetDescription)
        .addFunction("SetDescription", &Skill::SetDescription)
        .addFunction("GetCooldownTime", &Skill::GetCooldownTime)
        .addFunction("SetCooldownTime", &Skill::SetCooldownTime)*/
    );
}

}
