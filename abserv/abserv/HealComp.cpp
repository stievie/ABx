#include "stdafx.h"
#include "HealComp.h"
#include "Actor.h"
#include "Skill.h"

namespace Game {
namespace Components {

void HealComp::Healing(Actor* source, uint32_t index, int value)
{
    healings_.push_back({ source ? source->id_ : 0, index, value, Utils::Tick() });
    owner_.resourceComp_.SetHealth(Components::SetValueType::Increase, value);
}

void HealComp::Write(Net::NetworkMessage& message)
{
    if (healings_.size() == 0)
        return;
    for (const auto& d : healings_)
    {
        message.AddByte(AB::GameProtocol::GameObjectHealed);
        message.Add<uint32_t>(owner_.id_);
        message.Add<uint32_t>(static_cast<uint8_t>(d.actorId));
        message.Add<uint16_t>(static_cast<uint16_t>(d.index));
        message.Add<int16_t>(static_cast<int16_t>(d.value));
    }
    healings_.clear();
}

}
}
