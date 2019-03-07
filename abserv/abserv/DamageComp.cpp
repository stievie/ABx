#include "stdafx.h"
#include "DamageComp.h"
#include "Utils.h"
#include "Actor.h"

namespace Game {
namespace Components {

void DamageComp::ApplyDamage(DamageType type, int value)
{
    int damage = value;
    owner_.effectsComp_.GetDamage(type, damage);
    lastDamage_ = Utils::Tick();
    owner_.resourceComp_.SetHealth(SetValueType::Decrease, damage);
}

void DamageComp::Touch()
{
    lastDamage_ = Utils::Tick();
}

uint32_t DamageComp::NoDamageTime() const
{
    return Utils::TimePassed(lastDamage_);
}

}
}
