#include "stdafx.h"
#include "Creature.h"

namespace Game {

void Creature::RegisterLua(kaguya::State& state)
{
    GameObject::RegisterLua(state);
    state["Creature"].setClass(kaguya::UserdataMetatable<Creature, GameObject>()
        .addProperty("Speed", &Creature::GetSpeed, &Creature::SetSpeed)
        .addProperty("Energy", &Creature::GetEnergy, &Creature::SetEnergy)
        .addProperty("Health", &Creature::GetHealth, &Creature::SetHealth)
        .addProperty("Adrenaline", &Creature::GetAdrenaline, &Creature::SetAdrenaline)
        .addProperty("Overcast", &Creature::GetOvercast, &Creature::SetOvercast)
        .addProperty("Skills", &Creature::GetSkill)
    );
}

}
