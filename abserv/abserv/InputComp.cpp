#include "stdafx.h"
#include "InputComp.h"
#include "Actor.h"
#include "Game.h"
#include "NetworkMessage.h"

namespace Game {
namespace Components {

void InputComp::SelectObject(uint32_t sourceId, uint32_t targetId, Net::NetworkMessage& message)
{
    Actor* source = nullptr;
    if (sourceId == owner_.id_)
        source = &owner_;
    else
        source = dynamic_cast<Actor*>(owner_.GetGame()->GetObjectById(sourceId).get());

    if (source)
    {
        if (targetId == 0)
        {
            source->selectedObject_.reset();
            message.AddByte(AB::GameProtocol::GameObjectSelectTarget);
            message.Add<uint32_t>(source->id_);
            // Clear Target
            message.Add<uint32_t>(0);
        }
        else if (targetId != source->GetSelectedObjectId())
        {
            auto target = owner_.GetGame()->GetObjectById(targetId);
            if (target->selectable_)
            {
                source->selectedObject_ = owner_.GetGame()->GetObjectById(targetId);
                message.AddByte(AB::GameProtocol::GameObjectSelectTarget);
                message.Add<uint32_t>(source->id_);
                if (auto sel = source->selectedObject_.lock())
                {
                    sel->CallEvent<void(Actor*)>(EVENT_ON_SELECTED, source);
                    message.Add<uint32_t>(sel->id_);
                }
                else
                    // Clear Target
                    message.Add<uint32_t>(0);
            }
        }
    }
}

void InputComp::ClickObject(uint32_t sourceId, uint32_t targetId, Net::NetworkMessage&)
{
    Actor* source = nullptr;
    if (sourceId == owner_.id_)
        source = &owner_;
    else
        source = dynamic_cast<Actor*>(owner_.GetGame()->GetObjectById(sourceId).get());

    if (source)
    {
        auto clickedObj = owner_.GetGame()->GetObjectById(targetId);
        if (clickedObj)
            clickedObj->CallEvent<void(Actor*)>(EVENT_ON_CLICKED, source);
    }
}

void InputComp::FollowObject(uint32_t targetId, bool ping, Net::NetworkMessage&)
{
    if (!owner_.IsDead() && !owner_.IsKnockedDown())
    {
        auto target = owner_.GetGame()->GetObjectById(targetId);
        if (target)
        {
            bool succ = owner_.autorunComp_->Follow(target, ping);
            if (succ)
            {
                owner_.attackComp_->Cancel();
                owner_.followedObject_ = target;
                owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                owner_.autorunComp_->SetAutoRun(true);
            }
        }
#ifdef DEBUG_NAVIGATION
        else
        {
            LOG_WARNING << "InputType::Follow: object with ID not found: " << targetId << std::endl;
        }
#endif
    }
}

void InputComp::Update(uint32_t, Net::NetworkMessage& message)
{
    InputItem input;
    // Multiple inputs of the same type overwrite previous
    while (inputs_.Get(input))
    {
        switch (input.type)
        {
        case InputType::Move:
        {
            if (!owner_.IsImmobilized())
            {
                owner_.attackComp_->Cancel();
                owner_.moveComp_->moveDir_ = static_cast<AB::GameProtocol::MoveDirection>(input.data[InputDataDirection].GetInt());
                if (owner_.moveComp_->moveDir_ > AB::GameProtocol::MoveDirectionNone)
                {
                    owner_.skillsComp_->CancelWhenChangingState();
                    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                }
                else
                {
                    // Reset to Idle when neither moving nor turning
                    if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateMoving &&
                        owner_.moveComp_->turnDir_ == AB::GameProtocol::TurnDirectionNone)
                        owner_.stateComp_.Reset();
                }
                owner_.autorunComp_->Reset();
            }
            break;
        }
        case InputType::Turn:
        {
            if (!owner_.IsImmobilized())
            {
                owner_.attackComp_->Cancel();
                owner_.moveComp_->turnDir_ = static_cast<AB::GameProtocol::TurnDirection>(input.data[InputDataDirection].GetInt());
                if (owner_.moveComp_->turnDir_ > AB::GameProtocol::TurnDirectionNone)
                {
                    owner_.skillsComp_->CancelWhenChangingState();
                    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                }
                else
                {
                    if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateMoving &&
                        owner_.moveComp_->moveDir_ == AB::GameProtocol::MoveDirectionNone)
                        owner_.stateComp_.Reset();
                }
                owner_.autorunComp_->Reset();
            }
            break;
        }
        case InputType::Direction:
        {
            if (!owner_.IsImmobilized())
            {
                // No aurorunComp_.Reset() because manually setting the camera does not
                // stop autorunning
                if (!owner_.autorunComp_->IsAutoRun())
                    owner_.moveComp_->SetDirection(input.data[InputDataDirection].GetFloat());
            }
            break;
        }
        case InputType::SetState:
        {
            if (!owner_.IsImmobilized())
            {
                AB::GameProtocol::CreatureState state =
                    static_cast<AB::GameProtocol::CreatureState>(input.data[InputDataState].GetInt());
                owner_.stateComp_.SetState(state);
            }
            break;
        }
        case InputType::Goto:
        {
            if (!owner_.IsImmobilized())
            {
                owner_.autorunComp_->Reset();
                const Math::Vector3 dest = {
                    input.data[InputDataVertexX].GetFloat(),
                    input.data[InputDataVertexY].GetFloat(),
                    input.data[InputDataVertexZ].GetFloat()
                };
                const bool succ = owner_.autorunComp_->Goto(dest);
                if (succ)
                {
                    owner_.attackComp_->Cancel();
                    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                    owner_.autorunComp_->SetAutoRun(true);
                }
            }
            break;
        }
        case InputType::Follow:
        {
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            bool ping = input.data[InputDataPingTarget].GetBool();
            FollowObject(targetId, ping, message);
            break;
        }
        case InputType::Attack:
        {
            if (owner_.CanAttack() && !owner_.IsImmobilized())
            {
                if (auto target = owner_.selectedObject_.lock())
                {
                    auto actor = std::dynamic_pointer_cast<Actor>(target);
                    if (actor)
                    {
                        bool ping = input.data[InputDataPingTarget].GetBool();
                        owner_.attackComp_->Attack(actor, ping);
                    }
                }
            }
            break;
        }
        case InputType::UseSkill:
        {
            if (owner_.CanUseSkill() && !owner_.IsDead())
            {
                // The index of the skill in the users skill bar, 0 based
                int skillIndex = input.data[InputDataSkillIndex].GetInt();
                bool ping = input.data[InputDataPingTarget].GetBool();
                owner_.skillsComp_->UseSkill(skillIndex, ping);
            }
            break;
        }
        case InputType::Select:
        {
            // If a player could control a NPC (e.g. Hero), the player can select
            // targets for this NPC, so we also need the source ID.
            uint32_t sourceId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId2].GetInt());
            SelectObject(sourceId, targetId, message);
            break;
        }
        case InputType::ClickObject:
        {
            // If a player could control a NPC (e.g. Hero), the player can select
            // targets for this NPC, so we also need the source ID.
            uint32_t sourceId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId2].GetInt());
            ClickObject(sourceId, targetId, message);
            break;
        }
        case InputType::Cancel:
            // Cancel all
            owner_.skillsComp_->Cancel();
            owner_.attackComp_->Cancel();
            owner_.autorunComp_->Reset();
            owner_.stateComp_.Reset();
            break;
        case InputType::Command:
        {
            AB::GameProtocol::CommandTypes type = static_cast<AB::GameProtocol::CommandTypes>(input.data[InputDataCommandType].GetInt());
            const std::string& cmd = input.data[InputDataCommandData].GetString();
            owner_.CallEvent<void(AB::GameProtocol::CommandTypes,const std::string&,Net::NetworkMessage&)>(EVENT_ON_HANDLECOMMAND,
                type, cmd, message);
            break;
        }
        case InputType::None:
            break;
        }
    }
}

}
}
