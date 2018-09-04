#include "stdafx.h"
#include "Creature.h"
#include "EffectManager.h"
#include "Game.h"
#include <AB/ProtocolCodes.h>
#include "Logger.h"
#include "OctreeQuery.h"
#include "GameManager.h"
#include "MathUtils.h"

#include "DebugNew.h"

namespace Game {

void Creature::InitializeLua()
{
    GameManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
    luaInitialized_ = true;
}

void Creature::RegisterLua(kaguya::State& state)
{
    state["Creature"].setClass(kaguya::UserdataMetatable<Creature, GameObject>()
        .addFunction("GetLevel", &Creature::GetLevel)
        .addFunction("GetSkillBar", &Creature::GetSkillBar)
        .addFunction("GetSelectedObject", &Creature::GetSelectedObject)
        .addFunction("SetSelectedObject", &Creature::SetSelectedObject)

        .addFunction("GetSpeed", &Creature::GetSpeed)
        .addFunction("SetSpeed", &Creature::SetSpeed)
        .addFunction("GetEnergy", &Creature::GetEnergy)
        .addFunction("SetEnergy", &Creature::SetEnergy)
        .addFunction("AddEffect", &Creature::AddEffect)
        .addFunction("RemoveEffect", &Creature::RemoveEffect)

        .addFunction("GotoPosition", &Creature::_LuaGotoPosition)
        .addFunction("FollowObject", &Creature::FollowObject)
        .addFunction("GetState", &Creature::_LuaGetState)
    );
}

Creature::Creature() :
    GameObject(),
    moveComp_(*this),
    autorunComp_(*this),
    collisionComp_(*this),
    skills_(this),
    luaInitialized_(false)
{
    // Creature always collides
    static const Math::Vector3 CREATURTE_BB_MIN(-0.2f, 0.0f, -0.2f);
    static const Math::Vector3 CREATURTE_BB_MAX(0.2f, 1.7f, 0.2f);
    SetCollisionShape(
        std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(Math::ShapeTypeBoundingBox,
            CREATURTE_BB_MIN, CREATURTE_BB_MAX)
    );
    occluder_ = true;
}

void Creature::SetSelectedObject(std::shared_ptr<GameObject> object)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = GetId();    // Source
    if (object)
        data[InputDataObjectId2] = object->GetId();   // Target
    else
        data[InputDataObjectId2] = 0;   // Target
    inputs_.Add(InputType::Select, data);
}

void Creature::GotoPosition(const Math::Vector3& pos)
{
    Utils::VariantMap data;
    data[InputDataVertexX] = pos.x_;
    data[InputDataVertexY] = pos.y_;
    data[InputDataVertexZ] = pos.z_;
    inputs_.Add(InputType::Goto, data);
}

void Creature::FollowObject(std::shared_ptr<GameObject> object)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = object->id_;
    inputs_.Add(InputType::Follow, data);
}

bool Creature::Serialize(IO::PropWriteStream& stream)
{
    if (!GameObject::Serialize(stream))
        return false;
    stream.Write<uint32_t>(GetLevel());
    stream.Write<uint8_t>(GetSex());
    stream.Write<uint32_t>(GetProfIndex());
    stream.Write<uint32_t>(GetProf2Index());
    stream.Write<uint32_t>(GetModelIndex());
    return true;
}

void Creature::OnSelected(std::shared_ptr<Creature> selector)
{
    GameObject::OnSelected(selector);
    if (luaInitialized_)
        luaState_["onSelected"](selector);
}

void Creature::OnClicked(std::shared_ptr<Creature> selector)
{
    GameObject::OnSelected(selector);
    if (luaInitialized_)
        luaState_["onClicked"](selector);
}

void Creature::OnCollide(std::shared_ptr<Creature> other)
{
    GameObject::OnCollide(other);
    if (luaInitialized_)
        luaState_["onCollide"](other);
}

void Creature::AddEffect(std::shared_ptr<Creature> source, uint32_t index, uint32_t baseDuration)
{
    RemoveEffect(index);

    auto effect = EffectManager::Instance.Get(index);
    if (effect)
    {
        effects_.push_back(effect);
        effect->Start(source, GetThis<Creature>(), baseDuration);
    }
}

void Creature::DeleteEffect(uint32_t index)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [&](std::shared_ptr<Effect> const& current)
    {
        return current->data_.index == index;
    });
    if (it != effects_.end())
    {
        effects_.erase(it);
    }
}

void Creature::_LuaGotoPosition(float x, float y, float z)
{
    Math::Vector3 pos(x, y, z);
    if (Math::Equals(y, 0.0f))
    {
        pos.y_ = GetGame()->map_->GetTerrainHeight(pos);
    }
    GotoPosition(Math::Vector3(x, y, z));
}

int Creature::_LuaGetState()
{
    return static_cast<int>(stateComp_.GetState());
}

void Creature::RemoveEffect(uint32_t index)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [&](std::shared_ptr<Effect> const& current)
    {
        return current->data_.index == index;
    });
    if (it != effects_.end())
    {
        (*it)->Remove();
        DeleteEffect((*it)->data_.index);
    }
}

void Creature::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);

    Skill* skill = nullptr;
    int skillIndex = -1;
    moveComp_.turned_ = false;
    moveComp_.directionSet_ = false;

    InputItem input;

    stateComp_.Update(timeElapsed);

    // Multiple inputs of the same type overwrite previous
    while (inputs_.Get(input))
    {
        switch (input.type)
        {
        case InputType::Move:
        {
            moveComp_.moveDir_ = static_cast<AB::GameProtocol::MoveDirection>(input.data[InputDataDirection].GetInt());
            if (moveComp_.moveDir_ > AB::GameProtocol::MoveDirectionNone)
            {
                stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
            }
            else
            {
                // Reset to Idle when neither moving nor turning
                if (stateComp_.GetState() == AB::GameProtocol::CreatureStateMoving &&
                    moveComp_.turnDir_ == AB::GameProtocol::TurnDirectionNone)
                    stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            }
            autorunComp_.Reset();
            break;
        }
        case InputType::Turn:
        {
            moveComp_.turnDir_ = static_cast<AB::GameProtocol::TurnDirection>(input.data[InputDataDirection].GetInt());
            if (moveComp_.turnDir_ > AB::GameProtocol::TurnDirectionNone)
            {
                stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
            }
            else
            {
                if (stateComp_.GetState() == AB::GameProtocol::CreatureStateMoving &&
                    moveComp_.moveDir_ == AB::GameProtocol::MoveDirectionNone)
                    stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            }
            autorunComp_.Reset();
            break;
        }
        case InputType::Direction:
        {
            // No aurorunComp_.Reset() because manually setting the camera does not
            // stop autorunning
            if (!autorunComp_.autoRun_)
                moveComp_.SetDirection(input.data[InputDataDirection].GetFloat());
            break;
        }
        case InputType::Goto:
        {
            autorunComp_.Reset();
            const Math::Vector3 dest = {
                input.data[InputDataVertexX].GetFloat(),
                input.data[InputDataVertexY].GetFloat(),
                input.data[InputDataVertexZ].GetFloat()
            };
            bool succ = autorunComp_.Goto(dest);
            if (succ)
            {
                stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                autorunComp_.autoRun_ = true;
            }

            break;
        }
        case InputType::Follow:
        {
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            followedObject_ = GetGame()->GetObjectById(targetId);
            if (auto f = followedObject_.lock())
            {
                bool succ = autorunComp_.Follow(f);
                if (succ)
                {
                    stateComp_.SetState(AB::GameProtocol::CreatureStateMoving);
                    autorunComp_.autoRun_ = true;
                }
            }
#ifdef DEBUG_NAVIGATION
            else
            {
                LOG_WARNING << "InputType::Follow: object with ID not found: " << targetId << std::endl;
            }
#endif
            break;
        }
        case InputType::Attack:
            stateComp_.SetState(AB::GameProtocol::CreatureStateAttacking);
            break;
        case InputType::UseSkill:
        {
            skillIndex = static_cast<uint32_t>(input.data[InputDataSkillIndex].GetInt());
            skill = GetSkill(skillIndex);
            if (skill)
            {
                if (auto selObj = selectedObject_.lock())
                {
                    std::shared_ptr<Creature> target = selObj->GetThisDynamic<Creature>();
                    if (target)
                    {
                        // Can use skills only on Creatures not all GameObjects
                        skills_.UseSkill(skillIndex, target);
                        // These do not change the state
                        if (skill->IsChangingState())
                            stateComp_.SetState(AB::GameProtocol::CreatureStateUsingSkill);
                    }
                }
            }
            break;
        }
        case InputType::Select:
        {
            // If a player could control a NPC (e.g. Hero), the player can select
            // targets for this NPC, so we also need the source ID.
            uint32_t sourceId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId2].GetInt());

            Creature* source = nullptr;
            if (sourceId == id_)
                source = this;
            else
                source = dynamic_cast<Creature*>(GetGame()->GetObjectById(sourceId).get());

            if (source)
            {
                if (targetId != source->GetSelectedObjectId())
                {
                    source->selectedObject_ = GetGame()->GetObjectById(targetId);
                    message.AddByte(AB::GameProtocol::GameObjectSelectTarget);
                    message.Add<uint32_t>(source->id_);
                    if (auto sel = source->selectedObject_.lock())
                    {
                        sel->OnSelected(source->GetThis<Creature>());
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

            Creature* source = nullptr;
            if (sourceId == id_)
                source = this;
            else
                source = dynamic_cast<Creature*>(GetGame()->GetObjectById(sourceId).get());

            if (source)
            {
                auto clickedObj = GetGame()->GetObjectById(targetId);
                if (clickedObj)
                {
                    clickedObj->OnClicked(source->GetThis<Creature>());
                }
            }
            break;
        }
        case InputType::CancelSkill:
            if (stateComp_.GetState() == AB::GameProtocol::CreatureStateUsingSkill)
                stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            break;
        case InputType::CancelAttack:
            if (stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking)
                stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            break;
        case InputType::Command:
        {
            AB::GameProtocol::CommandTypes type = static_cast<AB::GameProtocol::CommandTypes>(input.data[InputDataCommandType].GetInt());
            const std::string& cmd = input.data[InputDataCommandData].GetString();
            HandleCommand(type, cmd, message);
            break;
        }
        }
    }

    if (stateComp_.IsStateChanged())
    {
        stateComp_.Apply();
#ifdef DEBUG_GAME
        LOG_DEBUG << "New state " << (int)stateComp_.GetState() << std::endl;
#endif
        message.AddByte(AB::GameProtocol::GameObjectStateChange);
        message.Add<uint32_t>(id_);
        message.AddByte(stateComp_.GetState());
    }
    if (moveComp_.speedDirty_)
    {
        moveComp_.speedDirty_ = false;
        message.AddByte(AB::GameProtocol::GameObjectMoveSpeedChange);
        message.Add<uint32_t>(id_);
        message.Add<float>(moveComp_.GetSpeedFactor());
    }

    switch (stateComp_.GetState())
    {
    case AB::GameProtocol::CreatureStateIdle:
        break;
    case AB::GameProtocol::CreatureStateMoving:
    {
        moveComp_.moved_ = false;
        moveComp_.Update(timeElapsed);
        autorunComp_.Update(timeElapsed);

        if (moveComp_.moved_)
        {
            message.AddByte(AB::GameProtocol::GameObjectPositionChange);
            message.Add<uint32_t>(id_);
            message.Add<float>(transformation_.position_.x_);
            message.Add<float>(transformation_.position_.y_);
            message.Add<float>(transformation_.position_.z_);
        }

        break;
    }
    case AB::GameProtocol::CreatureStateUsingSkill:
        break;
    case AB::GameProtocol::CreatureStateAttacking:
        break;
    case AB::GameProtocol::CreatureStateEmote:
        break;
    }

    // The rotation may change in 2 ways: Turn and SetWorldDirection
    if (moveComp_.turned_ || moveComp_.directionSet_)
    {
        message.AddByte(AB::GameProtocol::GameObjectRotationChange);
        message.Add<uint32_t>(id_);
        message.Add<float>(transformation_.rotation_);
        message.Add<uint8_t>(moveComp_.directionSet_ ? 1 : 0);
    }

    skills_.Update(timeElapsed);
    for (const auto& effect : effects_)
    {
        if (effect->cancelled_ || effect->ended_)
        {
            DeleteEffect(effect->data_.index);
            continue;
        }
        effect->Update(timeElapsed);
    }

}

}
