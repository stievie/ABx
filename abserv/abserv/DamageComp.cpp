#include "stdafx.h"
#include "DamageComp.h"
#include "Utils.h"
#include "Actor.h"

namespace Game {
namespace Components {

void DamageComp::ApplyDamage(DamageType type, int value, uint32_t skillIndex /* = 0 */)
{
    lastDamage_ = Utils::Tick();
    damages_.push_back({ type, value, skillIndex, lastDamage_ });
    owner_.resourceComp_.SetHealth(SetValueType::Decrease, value);
}

void DamageComp::Touch()
{
    lastDamage_ = Utils::Tick();
}

uint32_t DamageComp::NoDamageTime() const
{
    return Utils::TimePassed(lastDamage_);
}

void DamageComp::Write(Net::NetworkMessage& message)
{
    if (damages_.size() == 0)
        return;
    for (const auto& d : damages_)
    {
        message.AddByte(AB::GameProtocol::GameObjectDamaged);
        message.Add<uint32_t>(owner_.id_);
        message.Add<uint8_t>(static_cast<uint8_t>(d.type));
        message.Add<int16_t>(static_cast<uint16_t>(d.value));
        message.Add<uint32_t>(static_cast<uint32_t>(d.skillIndex));
    }
    damages_.clear();
}

}
}
