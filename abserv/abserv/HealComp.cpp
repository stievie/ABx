#include "stdafx.h"
#include "HealComp.h"
#include "Actor.h"
#include "Skill.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

void HealComp::Healing(Actor* source, uint32_t index, int value)
{
    if (owner_.IsDead())
        return;

    healings_.push_back({ source ? source->id_ : 0, index, value, Utils::Tick() });
    owner_.resourceComp_->SetHealth(Components::SetValueType::Increase, value);
}

void HealComp::Write(Net::NetworkMessage& message)
{
    if (healings_.size() == 0)
        return;
    for (const auto& d : healings_)
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectHealed);
        AB::Packets::Server::ObjectHealed packet = {
            owner_.id_,
            d.actorId,
            static_cast<uint16_t>(d.index),
            static_cast<int16_t>(d.value)
        };
        AB::Packets::Add(packet, message);
    }
    healings_.clear();
}

}
}
