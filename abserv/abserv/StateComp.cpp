#include "stdafx.h"
#include "StateComp.h"
#include "GameObject.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

void StateComp::SetState(AB::GameProtocol::CreatureState state, bool apply /* = false */)
{
    if (state != newState_)
    {
        newState_ = state;
        if (apply)
            Apply();
    }
}

bool StateComp::KnockDown(uint32_t time)
{
    if (IsKnockedDown())
        return false;
    knockdownEndTime_ = Utils::Tick() + time;
    SetState(AB::GameProtocol::CreatureState::KnockedDown);
    return true;
}

void StateComp::Apply()
{
    lastStateChange_ = Utils::Tick();
    currentState_ = newState_;
}

void StateComp::Reset()
{
    SetState(AB::GameProtocol::CreatureState::Idle);
}

void StateComp::Update(uint32_t)
{
    if (IsKnockedDown())
    {
        if (knockdownEndTime_ <= Utils::Tick())
        {
            SetState(AB::GameProtocol::CreatureState::Idle);
        }
        return;
    }
    if ((currentState_ > AB::GameProtocol::CreatureState::__EmoteStart &&
        currentState_ < AB::GameProtocol::CreatureState::__EmoteEnd)
        && lastStateChange_ + 4000 < Utils::Tick())
    {
        // Reset some emotes after 4 seconds
        SetState(AB::GameProtocol::CreatureState::Idle);
    }
}

void StateComp::Write(Net::NetworkMessage& message)
{
    if (IsStateChanged())
    {
        Apply();
#ifdef DEBUG_GAME
        LOG_DEBUG << "New state of " << owner_ << ": " << (int)GetState() << std::endl;
#endif
        message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectStateChange);
        AB::Packets::Server::ObjectStateChanged packet = {
            owner_.id_,
            static_cast<uint8_t>(GetState())
        };
        AB::Packets::Add(packet, message);
    }
}

}
}
