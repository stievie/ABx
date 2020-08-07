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
        if (owner_.IsInRange(range, interactingWith.get()))
        {
            npc.CallEvent<void(Actor*)>(EVENT_ON_INTERACT, &owner_);
            interacting_ = false;
        }
        return;
    }
}

void InteractionComp::Interact(bool ping)
{
    auto* interactingWith = owner_.selectionComp_->GetSelectedObject();
    if (!interactingWith)
        return;
    auto interactingWithSp = interactingWith->GetPtr<Actor>();
    if (!interactingWithSp)
        return;

    interactingWith_ = interactingWithSp;

    if (AB::Entities::IsOutpost(owner_.GetGame()->data_.type))
    {
        if (Is<Player>(interactingWith))
        {
            owner_.autorunComp_->Follow(interactingWithSp, ping);
            interacting_ = true;
            return;
        }
        if (Is<Npc>(interactingWith))
        {
            auto& npc = To<Npc>(*interactingWith);
            Ranges range = npc.GetInteractionRange();
            if (!owner_.IsInRange(range, interactingWith))
            {
                if (owner_.FollowObject(interactingWith, false))
                {
                    interacting_ = true;
                    if (ping)
                        owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT, interactingWith->id_, AB::GameProtocol::ObjectCallType::TalkingTo, 0);
                }
            }
            else
            {
                interacting_ = true;
                if (ping)
                    owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT, interactingWith->id_, AB::GameProtocol::ObjectCallType::TalkingTo, 0);
            }
            return;
        }
        return;
    }

    if (owner_.IsAlly(To<Actor>(interactingWith)))
    {
        owner_.FollowObject(interactingWith, ping);
        interacting_ = false;
        return;
    }
    if (owner_.IsEnemy(To<Actor>(interactingWith)))
    {
        owner_.Attack(To<Actor>(interactingWith), ping);
        interacting_ = false;
        return;
    }
}

}
}
