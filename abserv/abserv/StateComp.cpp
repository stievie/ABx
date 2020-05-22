/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "StateComp.h"
#include "GameObject.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include "Actor.h"

namespace Game {
namespace Components {

void StateComp::SetState(AB::GameProtocol::CreatureState state, bool apply /* = false */)
{
    if (state != newState_)
    {
        auto oldState = newState_;
        newState_ = state;
        if (apply)
            Apply();
        owner_.CallEvent<void(AB::GameProtocol::CreatureState, AB::GameProtocol::CreatureState)>(EVENT_ON_STATECHANGE, oldState, newState_);
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

void StateComp::GroupMaskChanged()
{
    groupMaskChanged_ = true;
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
    if (groupMaskChanged_ && Is<Actor>(owner_))
    {
        message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectGroupMaskChanged);
        AB::Packets::Server::ObjectGroupMaskChanged packet = {
            owner_.id_,
            To<Actor>(owner_).groupMask_
        };
        AB::Packets::Add(packet, message);
        groupMaskChanged_ = false;
    }
}

}
}
