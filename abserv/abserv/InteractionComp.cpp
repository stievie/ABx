/**
 * Copyright 2020 Stefan Ascher
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

#include "InteractionComp.h"
#include "Player.h"
#include "SelectionComp.h"
#include "Npc.h"
#include "ItemDrop.h"

namespace Game {
namespace Components {

InteractionComp::InteractionComp(Player& owner) :
    owner_(owner)
{
    owner_.SubscribeEvent<void(void)>(EVENT_ON_CANCELALL, std::bind(&InteractionComp::OnCancelAll, this));
}

void InteractionComp::OnCancelAll()
{
    interacting_ = false;
}

void InteractionComp::Update(uint32_t)
{
    if (!interacting_)
        return;

    auto interactingWith = interactingWith_.lock();
    if (!interactingWith)
        return;
    if (Is<Npc>(*interactingWith))
    {
        auto& npc = To<Npc>(*interactingWith);
        Ranges range = npc.GetInteractionRange();
        if (npc.IsInRange(range, &owner_))
        {
            npc.CallEvent<void(Actor*)>(EVENT_ON_INTERACT, &owner_);
            interacting_ = false;
        }
        return;
    }

    if (Is<ItemDrop>(*interactingWith))
    {
        if (interactingWith->IsInRange(Ranges::Adjecent, &owner_))
        {
            To<ItemDrop>(*interactingWith).PickUp(&owner_);
            interacting_ = false;
        }
        return;
    }

    if (Is<Player>(*interactingWith))
    {
        if (interactingWith->IsInRange(Ranges::Adjecent, &owner_))
        {
            interacting_ = false;
        }
        return;
    }
}

void InteractionComp::Interact(bool suppress, bool ping)
{
    auto* interactingWith = owner_.selectionComp_->GetSelectedObject();
    if (!interactingWith)
        return;
    auto interactingWithSp = interactingWith->GetPtr<GameObject>();
    if (!interactingWithSp)
        return;

    interactingWith_ = interactingWithSp;

    if (AB::Entities::IsOutpost(owner_.GetGame()->data_.type))
    {
        if (Is<Player>(interactingWith))
        {
            if (!suppress)
            {
                owner_.FollowObject(interactingWith, ping, RANGE_TOUCH);
                interacting_ = true;
                return;
            }
            if (ping)
                owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT, interactingWith->id_,
                    AB::GameProtocol::ObjectCallType::Follow, 0);
            return;
        }
        if (Is<Npc>(interactingWith))
        {
            if (!suppress)
            {
                auto& npc = To<Npc>(*interactingWith);
                Ranges range = npc.GetInteractionRange();
                if (!owner_.IsInRange(range, interactingWith))
                {
                    if (owner_.FollowObject(interactingWith, false, RANGE_TOUCH))
                    {
                        interacting_ = true;
                        if (ping)
                            owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
                                interactingWith->id_, AB::GameProtocol::ObjectCallType::TalkingTo, 0);
                    }
                    return;
                }
                interacting_ = true;
                if (ping)
                    owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
                        interactingWith->id_, AB::GameProtocol::ObjectCallType::TalkingTo, 0);
                return;
            }
            if (ping)
                owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
                    interactingWith->id_, AB::GameProtocol::ObjectCallType::TalkingTo, 0);
        }
        return;
    }

    if (Is<ItemDrop>(interactingWith))
    {
        if (suppress)
        {
            if (ping)
                owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
                    interactingWith->id_, AB::GameProtocol::ObjectCallType::PickingUp, 0);
            return;
        }

        if (!owner_.IsInRange(Ranges::Adjecent, interactingWith))
        {
            if (owner_.FollowObject(interactingWith, false, RANGE_TOUCH))
            {
                interacting_ = true;
                if (ping)
                    owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
                        interactingWith->id_, AB::GameProtocol::ObjectCallType::PickingUp, 0);
            }
            return;
        }

        interacting_ = true;
        if (ping)
            owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
                interactingWith->id_, AB::GameProtocol::ObjectCallType::PickingUp, 0);
        return;
    }

    if (Is<Actor>(interactingWith))
    {
        if (owner_.IsAlly(To<Actor>(interactingWith)))
        {
            if (!suppress)
            {
                owner_.FollowObject(interactingWith, ping);
                interacting_ = false;
            }
            else
            {
                if (ping)
                    owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
                        interactingWith->id_, AB::GameProtocol::ObjectCallType::Follow, 0);
            }
            return;
        }
        if (owner_.IsEnemy(To<Actor>(interactingWith)))
        {
            if (!suppress)
            {
                owner_.Attack(To<Actor>(interactingWith), ping);
                interacting_ = false;
            }
            else
            {
                if (ping)
                    owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
                        interactingWith->id_, AB::GameProtocol::ObjectCallType::Target, 0);
            }
            return;
        }
    }
}

}
}
