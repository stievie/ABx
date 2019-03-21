#include "stdafx.h"
#include "DamageComp.h"
#include "Utils.h"
#include "Actor.h"
#include "Subsystems.h"
#include "Random.h"
#include "Utils.h"
#include "Mechanic.h"

namespace Game {
namespace Components {

void DamageComp::ApplyDamage(Actor* source, uint32_t index, DamageType type, int value, float penetration)
{
    lastDamage_ = Utils::Tick();
    // Get a random pos where to hit
    const DamagePos pos = GetDamagePos();
    // Get the armor effect at this pos with the given damage type and armor penetration
    const float am = owner_.GetArmorEffect(type, pos, penetration);
    const int realValue = static_cast<int>(static_cast<float>(value) * am);
    damages_.push_back({ type, pos, realValue, source ? source->id_ : 0, index, lastDamage_ });
    owner_.resourceComp_.SetHealth(SetValueType::Decrease, value);
}

int DamageComp::DrainLife(Actor* source, uint32_t index, int value)
{
    const int currLife = owner_.resourceComp_.GetHealth();
    const int result = Math::Clamp(value, 0, currLife);
    lastDamage_ = Utils::Tick();
    damages_.push_back({ DamageType::LifeDrain, DamagePos::NoPos, result, source ? source->id_ : 0, index, lastDamage_ });
    owner_.resourceComp_.SetHealth(Components::SetValueType::Absolute, currLife - result);
    return result;
}

void DamageComp::Touch()
{
    lastDamage_ = Utils::Tick();
}

DamagePos DamageComp::GetDamagePos() const
{
    // https://stackoverflow.com/questions/3655430/selection-based-on-percentage-weighting
    const float rnd = GetSubsystem<Crypto::Random>()->GetFloat();
    float percent = 0.0f;
    for (size_t i = 0; i < Utils::CountOf(DamagePosChances); ++i)
    {
        percent += DamagePosChances[i];
        if (rnd < percent)
            return static_cast<DamagePos>(i);
    }

    return DamagePos::Chest;
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
        message.Add<uint16_t>(static_cast<uint16_t>(d.index));
        message.Add<uint8_t>(static_cast<uint8_t>(d.type));
        message.Add<int16_t>(static_cast<int16_t>(d.value));
    }
    damages_.clear();
}

}
}
