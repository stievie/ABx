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

#include "stdafx.h"
#include "SelectionComp.h"
#include <AB/ProtocolCodes.h>
#include "Actor.h"
#include "Game.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <abshared/Mechanic.h>

namespace Game {
namespace Components {

SelectionComp::~SelectionComp() = default;

void SelectionComp::Write(Net::NetworkMessage& message)
{
    if (prevObjectId_ == currObjectId_)
        return;

    message.AddByte(AB::GameProtocol::ServerPacketType::GameObjectSelectTarget);
    AB::Packets::Server::ObjectTargetSelected packet = {
        owner_.id_,
        currObjectId_
    };
    AB::Packets::Add(packet, message);

    prevObjectId_ = currObjectId_;
}

bool SelectionComp::SelectObject(uint32_t targetId)
{
    auto* target = owner_.GetGame()->GetObject<GameObject>(targetId);
    // 0 = unselecting object
    if ((targetId != 0) && (!target || !target->selectable_))
        return false;
    if (currObjectId_ == targetId)
        return true;

    if (targetId != 0)
    {
        if (owner_.GetDistance(target) > RANGE_SELECT)
            return false;
    }

    currObjectId_ = targetId;
    if (target)
        target->CallEvent<void(Actor*)>(EVENT_ON_SELECTED, &owner_);
    return true;
}

bool SelectionComp::ClickObject(uint32_t targetId)
{
    auto* clickedObj = owner_.GetGame()->GetObject<GameObject>(targetId);
    if (clickedObj)
    {
        clickedObj->CallEvent<void(Actor*)>(EVENT_ON_CLICKED, &owner_);
        return true;
    }
    return false;
}

GameObject* SelectionComp::GetSelectedObject() const
{
    return owner_.GetGame()->GetObject<GameObject>(currObjectId_);
}

}
}
