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

void InteractionComp::UpdateGeneric()
{
    auto interactingWith = target_.lock();
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

void InteractionComp::UpdateSkill()
{
    auto interactingWith = target_.lock();
    if (!interactingWith)
        return;

    if (Is<Actor>(*interactingWith))
    {
        if (interactingWith->IsInRange(skillRange_, &owner_))
        {
            owner_.UseSkill(skillIndex_, false);
            interacting_ = false;
        }
        return;
    }
}

void InteractionComp::Update(uint32_t)
{
    if (!interacting_)
        return;

    if (type_ == Type::Generic)
        return UpdateGeneric();

    return UpdateSkill();
}

void InteractionComp::Interact(bool suppress, bool ping)
{
    auto* interactingWith = owner_.selectionComp_->GetSelectedObject();
    if (!interactingWith)
        return;
    auto interactingWithSp = interactingWith->GetPtr<GameObject>();
    if (owner_.IsImmobilized() || owner_.IsDead())
        return;

    target_ = interactingWithSp;
    type_ = Type::Generic;

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

void InteractionComp::UseSkill(bool suppress, int skillIndex, bool ping)
{
    if (skillIndex < 0 || skillIndex >= PLAYER_MAX_SKILLS)
        return;
    if (owner_.IsImmobilized() || owner_.IsDead())
        return;

    auto* interactingWith = owner_.selectionComp_->GetSelectedObject();
    if (interactingWith)
        target_ = interactingWith->GetPtr<GameObject>();
    else
        target_.reset();

    skillIndex_ = skillIndex;
    SkillBar* sb = owner_.GetSkillBar();
    auto skill = sb->GetSkill(skillIndex);

    if (suppress)
    {
        owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
            interactingWith ? interactingWith->id_ : 0,
            AB::GameProtocol::ObjectCallType::UseSkill, skillIndex);
        return;
    }

    if (!skill)
    {
        // Will send an error to the player
        owner_.UseSkill(skillIndex, ping);
        return;
    }
    skillRange_ = skill->GetRange();
    if (!skill->NeedsTarget())
    {
        // Can use it right away
        owner_.UseSkill(skillIndex, ping);
        return;
    }
    if (!interactingWith)
    {
        // Needs a target but we don't have one -> send an error to the player
        owner_.UseSkill(skillIndex, ping);
        return;
    }

    if (ping)
        owner_.CallEvent<void(uint32_t, AB::GameProtocol::ObjectCallType, int)>(EVENT_ON_PINGOBJECT,
            interactingWith ? interactingWith->id_ : 0,
            AB::GameProtocol::ObjectCallType::UseSkill, skillIndex);
    owner_.FollowObject(interactingWith, false, RangeDistances[static_cast<size_t>(skillRange_)]);
    interacting_ = true;
    type_ = Type::Skill;
}

}
}
