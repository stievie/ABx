#include "stdafx.h"
#include "Skill.h"

namespace Game {

void Skill::RegisterLua(kaguya::State& state)
{
    state["Skill"].setClass(kaguya::UserdataMetatable<Skill>()
        /*        .addFunction("GetName", &Skill::GetName)
        .addFunction("SetName", &Skill::SetName)
        .addFunction("GetDescription", &Skill::GetDescription)
        .addFunction("SetDescription", &Skill::SetDescription)
        .addFunction("GetCooldownTime", &Skill::GetCooldownTime)
        .addFunction("SetCooldownTime", &Skill::SetCooldownTime)*/
    );
}

Skill::Skill()
{
}


Skill::~Skill()
{
}

}
