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

inline uint32_t DamageComp::NoDamageTime() const
{
    return static_cast<uint32_t>(Utils::AbTick() - lastDamage_);
}

}
}
