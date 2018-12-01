#include "stdafx.h"
#include "InputComp.h"
#include "Actor.h"
#include "Game.h"

namespace Game {
namespace Components {

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
            if (!owner_.IsDead())
            {
                owner_.moveComp_.moveDir_ = static_cast<AB::GameProtocol::MoveDirection>(input.data[InputDataDirection].GetInt());
                if (owner_.moveComp_.moveDir_ > AB::GameProtocol::MoveDirectionNone)
                {
                    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                }
                else
                {
                    // Reset to Idle when neither moving nor turning
                    if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateMoving &&
                        owner_.moveComp_.turnDir_ == AB::GameProtocol::TurnDirectionNone)
                        owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
                }
                owner_.autorunComp_.Reset();
            }
            break;
        }
        case InputType::Turn:
        {
            if (!owner_.IsDead())
            {
                owner_.moveComp_.turnDir_ = static_cast<AB::GameProtocol::TurnDirection>(input.data[InputDataDirection].GetInt());
                if (owner_.moveComp_.turnDir_ > AB::GameProtocol::TurnDirectionNone)
                {
                    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                }
                else
                {
                    if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateMoving &&
                        owner_.moveComp_.moveDir_ == AB::GameProtocol::MoveDirectionNone)
                        owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
                }
                owner_.autorunComp_.Reset();
            }
            break;
        }
        case InputType::Direction:
        {
            if (!owner_.IsDead())
            {
                // No aurorunComp_.Reset() because manually setting the camera does not
                // stop autorunning
                if (!owner_.autorunComp_.autoRun_)
                    owner_.moveComp_.SetDirection(input.data[InputDataDirection].GetFloat());
            }
            break;
        }
        case InputType::SetState:
        {
            if (!owner_.IsDead())
            {
                AB::GameProtocol::CreatureState state =
                    static_cast<AB::GameProtocol::CreatureState>(input.data[InputDataState].GetInt());
                owner_.stateComp_.SetState(state);
            }
            break;
        }
        case InputType::Goto:
        {
            if (!owner_.IsDead())
            {
                owner_.autorunComp_.Reset();
                const Math::Vector3 dest = {
                    input.data[InputDataVertexX].GetFloat(),
                    input.data[InputDataVertexY].GetFloat(),
                    input.data[InputDataVertexZ].GetFloat()
                };
                bool succ = owner_.autorunComp_.Goto(dest);
                if (succ)
                {
                    owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                    owner_.autorunComp_.autoRun_ = true;
                }
            }
            break;
        }
        case InputType::Follow:
        {
            if (!owner_.IsDead())
            {
                uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
                owner_.followedObject_ = owner_.GetGame()->GetObjectById(targetId);
                if (auto f = owner_.followedObject_.lock())
                {
                    bool succ = owner_.autorunComp_.Follow(f);
                    if (succ)
                    {
                        owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                        owner_.autorunComp_.autoRun_ = true;
                    }
                }
#ifdef DEBUG_NAVIGATION
                else
                {
                    LOG_WARNING << "InputType::Follow: object with ID not found: " << targetId << std::endl;
                }
#endif
            }
            break;
        }
        case InputType::Attack:
            if (!owner_.IsDead())
                owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateAttacking);
            break;
        case InputType::UseSkill:
        {
            if (!owner_.IsDead())
            {
                int skillIndex = input.data[InputDataSkillIndex].GetInt();
#ifdef DEBUG_GAME
                LOG_DEBUG << owner_.GetName() << " is using skill " << skillIndex << std::endl;
#endif
                owner_.skillsComp_.UseSkill(skillIndex);
            }
            break;
        }
        case InputType::Select:
        {
            // If a player could control a NPC (e.g. Hero), the player can select
            // targets for this NPC, so we also need the source ID.
            uint32_t sourceId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId2].GetInt());

            Actor* source = nullptr;
            if (sourceId == owner_.id_)
                source = &owner_;
            else
                source = dynamic_cast<Actor*>(owner_.GetGame()->GetObjectById(sourceId).get());

            if (source)
            {
                if (targetId != source->GetSelectedObjectId())
                {
                    source->selectedObject_ = owner_.GetGame()->GetObjectById(targetId);
                    message.AddByte(AB::GameProtocol::GameObjectSelectTarget);
                    message.Add<uint32_t>(source->id_);
                    if (auto sel = source->selectedObject_.lock())
                    {
                        sel->OnSelected(source->GetThis<Actor>());
                        message.Add<uint32_t>(sel->id_);
                    }
                    else
                        // Clear Target
                        message.Add<uint32_t>(0);
                }
            }
            break;
        }
        case InputType::ClickObject:
        {
            // If a player could control a NPC (e.g. Hero), the player can select
            // targets for this NPC, so we also need the source ID.
            uint32_t sourceId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId2].GetInt());

            Actor* source = nullptr;
            if (sourceId == owner_.id_)
                source = &owner_;
            else
                source = dynamic_cast<Actor*>(owner_.GetGame()->GetObjectById(sourceId).get());

            if (source)
            {
                auto clickedObj = owner_.GetGame()->GetObjectById(targetId);
                if (clickedObj)
                {
                    clickedObj->OnClicked(source->GetThis<Actor>());
                }
            }
            break;
        }
        case InputType::Cancel:
            if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateUsingSkill)
                owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            else if (owner_.stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking)
                owner_.stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            break;
        case InputType::Command:
        {
            AB::GameProtocol::CommandTypes type = static_cast<AB::GameProtocol::CommandTypes>(input.data[InputDataCommandType].GetInt());
            const std::string& cmd = input.data[InputDataCommandData].GetString();
            owner_.HandleCommand(type, cmd, message);
            break;
        }
        }
    }
}

}
}
