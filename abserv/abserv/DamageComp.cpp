#include "stdafx.h"
#include "DamageComp.h"
#include "Utils.h"
#include "Actor.h"

namespace Game {
namespace Components {

void DamageComp::ApplyDamage(Actor* source, Skill* skill, DamageType type, int value)
{
    lastDamage_ = Utils::Tick();
    damages_.push_back({ type, value, source ? source->id_ : 0, skill ? static_cast<int>(skill->data_.index) : -1, lastDamage_ });
    owner_.resourceComp_.SetHealth(SetValueType::Decrease, value);
}

int DamageComp::DrainLife(Actor* source, Skill* skill, int value)
{
    int currLife = owner_.resourceComp_.GetHealth();
    int result = Math::Clamp(value, 0, currLife);
    lastDamage_ = Utils::Tick();
    damages_.push_back({ DamageType::LifeDrain, result, source ? source->id_ : 0, skill ? static_cast<int>(skill->data_.index) : -1, lastDamage_ });
    owner_.resourceComp_.SetHealth(Components::SetValueType::Absolute, currLife - result);
    return result;
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
        message.Add<uint32_t>(d.actorId);
        message.Add<uint16_t>(static_cast<int16_t>(d.skillIndex));
        message.Add<uint8_t>(static_cast<uint8_t>(d.type));
        message.Add<int16_t>(static_cast<int16_t>(d.value));
    }
    damages_.clear();
}

}
}
