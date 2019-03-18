#include "stdafx.h"
#include "HealComp.h"
#include "Actor.h"

namespace Game {
namespace Components {

void HealComp::Healing(Actor* source, int value)
{
    healings_.push_back({ source ? source->id_ : 0, value, Utils::Tick() });
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
        message.Add<int16_t>(static_cast<uint16_t>(d.value));
    }
    healings_.clear();
}

}
}
