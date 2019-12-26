#include "stdafx.h"
#include "SelectionComp.h"
#include <AB/ProtocolCodes.h>
#include "Actor.h"
#include "Game.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

SelectionComp::~SelectionComp() = default;

void SelectionComp::Write(Net::NetworkMessage& message)
{
    if (prevObjectId_ == currObjectId_)
        return;

    message.AddByte(AB::GameProtocol::GameObjectSelectTarget);
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
    if (!target || !target->selectable_)
        return false;
    if (currObjectId_ == targetId)
        return true;

    currObjectId_ = targetId;
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
