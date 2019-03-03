#include "stdafx.h"
#include "DamageComp.h"
#include "Utils.h"
#include "Actor.h"

namespace Game {
namespace Components {

void DamageComp::ApplyDamage(DamageType type, int value)
{
    int damage = owner_.effectsComp_.GetDamage(type, value);
    lastDamage_ = Utils::AbTick();
    owner_.resourceComp_.SetHealth(SetValueType::Decrease, damage);
}

void DamageComp::Touch()
{
    lastDamage_ = Utils::AbTick();
}

uint32_t DamageComp::NoDamageTime() const
{
    return Utils::TimePassed(lastDamage_);
}

}
}
