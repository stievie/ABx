#include "stdafx.h"
#include "Actor.h"
#include "EffectManager.h"
#include "Game.h"
#include <AB/ProtocolCodes.h>
#include "Logger.h"
#include "OctreeQuery.h"
#include "GameManager.h"
#include "MathUtils.h"
#include "ScriptManager.h"
#include "ConfigManager.h"

#include "DebugNew.h"

namespace Game {

void Actor::RegisterLua(kaguya::State& state)
{
    state["Actor"].setClass(kaguya::UserdataMetatable<Actor, GameObject>()
        .addFunction("IsTrigger", &Actor::IsTrigger)
        .addFunction("SetTrigger", &Actor::SetTrigger)

        .addFunction("GetLevel", &Actor::GetLevel)
        .addFunction("GetSkillBar", &Actor::GetSkillBar)
        .addFunction("GetSelectedObject", &Actor::GetSelectedObject)
        .addFunction("SetSelectedObject", &Actor::SetSelectedObject)
        .addFunction("UseSkill", &Actor::UseSkill)
        .addFunction("GetCurrentSkill", &Actor::GetCurrentSkill)

        .addFunction("IsUndestroyable", &Actor::IsUndestroyable)
        .addFunction("SetUndestroyable", &Actor::SetUndestroyable)
        .addFunction("GetSpeed", &Actor::GetSpeed)
        .addFunction("SetSpeed", &Actor::SetSpeed)
        .addFunction("AddEffect", &Actor::_LuaAddEffect)
        .addFunction("RemoveEffect", &Actor::_LuaRemoveEffect)

        .addFunction("GotoPosition", &Actor::_LuaGotoPosition)
        .addFunction("FollowObject", &Actor::_LuaFollowObject)
        .addFunction("GetState", &Actor::_LuaGetState)
        .addFunction("SetState", &Actor::_LuaSetState)

        .addFunction("GetHomePos", &Actor::_LuaGetHomePos)
        .addFunction("SetHomePos", &Actor::_LuaSetHomePos)
        .addFunction("GotoHomePos", &Actor::GotoHomePos)
        .addFunction("IsDead", &Actor::IsDead)
        .addFunction("Die", &Actor::Die)
        .addFunction("GetActorsInRange", &Actor::_LuaGetActorsInRange)
    );
}

Actor::Actor() :
    GameObject(),
    skills_(this),
    moveComp_(*this),
    autorunComp_(*this),
    collisionComp_(*this),
    resourceComp_(*this),
    attackComp_(*this),
    effectsComp_(*this),
    equipComp_(*this),
    skillsComp_(*this),
    undestroyable_(false),
    retriggerTimeout_(1000)
{
    // Actor always collides
    static const Math::Vector3 CREATURTE_BB_MIN(-0.2f, 0.0f, -0.2f);
    static const Math::Vector3 CREATURTE_BB_MAX(0.2f, 1.7f, 0.2f);
    SetCollisionShape(
        std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(Math::ShapeTypeBoundingBox,
            CREATURTE_BB_MIN, CREATURTE_BB_MAX)
    );
    occluder_ = true;
    resourceComp_.SetMaxHealth(500);
    resourceComp_.SetMaxEnergy(50);
    resourceComp_.SetHealth(Components::SetValueType::Absolute, 500);
    resourceComp_.SetEnergy(Components::SetValueType::Absolute, 50);
}

bool Actor::GotoHomePos()
{
    if (!homePos_.Equals(transformation_.position_))
    {
        GotoPosition(homePos_);
        return true;
    }
    return false;
}

void Actor::SetSelectedObject(std::shared_ptr<GameObject> object)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = GetId();    // Source
    if (object)
        data[InputDataObjectId2] = object->GetId();   // Target
    else
        data[InputDataObjectId2] = 0;   // Target
    inputs_.Add(InputType::Select, data);
}

void Actor::GotoPosition(const Math::Vector3& pos)
{
    Utils::VariantMap data;
    data[InputDataVertexX] = pos.x_;
    data[InputDataVertexY] = pos.y_;
    data[InputDataVertexZ] = pos.z_;
    inputs_.Add(InputType::Goto, data);
}

void Actor::FollowObject(std::shared_ptr<GameObject> object)
{
    FollowObject(object->id_);
}

void Actor::FollowObject(uint32_t objectId)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = objectId;
    inputs_.Add(InputType::Follow, data);
}

void Actor::UseSkill(uint32_t index)
{
    Utils::VariantMap data;
    data[InputDataSkillIndex] = static_cast<uint8_t>(index);
    inputs_.Add(InputType::UseSkill, data);
}

void Actor::_LuaSetState(int state)
{
    Utils::VariantMap data;
    data[InputDataState] = static_cast<uint8_t>(state);
    inputs_.Add(InputType::SetState, data);
}

void Actor::_LuaSetHomePos(float x, float y, float z)
{
    homePos_ = { x, y, z };
    if (Math::Equals(homePos_.y_, 0.0f))
    {
        homePos_.y_ = GetGame()->map_->GetTerrainHeight(homePos_);
    }
}

std::vector<float> Actor::_LuaGetHomePos()
{
    std::vector<float> result;
    result.push_back(homePos_.x_);
    result.push_back(homePos_.y_);
    result.push_back(homePos_.z_);
    return result;
}

void Actor::_LuaFollowObject(std::shared_ptr<GameObject> object)
{
    FollowObject(object);
}

std::vector<std::shared_ptr<Actor>> Actor::_LuaGetActorsInRange(Ranges range)
{
    std::vector<std::shared_ptr<Actor>> result;
    VisitInRange(range, [&](GameObject* const o)
    {
        AB::GameProtocol::GameObjectType t = o->GetType();
        if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
            result.push_back(o->GetThisDynamic<Actor>());
    });
    return result;
}

void Actor::_LuaAddEffect(std::shared_ptr<Actor> source, uint32_t index, uint32_t baseDuration)
{
    effectsComp_.AddEffect(source, index, baseDuration);
}

void Actor::_LuaRemoveEffect(uint32_t index)
{
    effectsComp_.RemoveEffect(index);
}

bool Actor::Serialize(IO::PropWriteStream& stream)
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

void Actor::WriteSpawnData(Net::NetworkMessage& msg)
{
    msg.Add<uint32_t>(id_);
    msg.Add<float>(transformation_.position_.x_);
    msg.Add<float>(transformation_.position_.y_);
    msg.Add<float>(transformation_.position_.z_);
    msg.Add<float>(transformation_.rotation_);
    msg.Add<float>(transformation_.scale_.x_);
    msg.Add<float>(transformation_.scale_.y_);
    msg.Add<float>(transformation_.scale_.z_);
    msg.Add<uint8_t>(undestroyable_ ? 1 : 0);
    msg.Add<uint8_t>(stateComp_.GetState());
    msg.Add<float>(GetSpeed());
    IO::PropWriteStream data;
    size_t dataSize;
    Serialize(data);
    const char* cData = data.GetStream(dataSize);
    msg.AddString(std::string(cData, dataSize));
    resourceComp_.Write(msg, true);
}

Skill* Actor::GetCurrentSkill() const
{
    return skills_.GetCurrentSkill();
}

void Actor::_LuaGotoPosition(float x, float y, float z)
{
    Math::Vector3 pos(x, y, z);
    if (Math::Equals(y, 0.0f))
    {
        pos.y_ = GetGame()->map_->GetTerrainHeight(pos);
    }
    GotoPosition(pos);
}

int Actor::_LuaGetState()
{
    return static_cast<int>(stateComp_.GetState());
}

void Actor::UpdateRanges()
{
    ranges_.clear();
    std::vector<GameObject*> res;

    static const float RANGE_AGGRO = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeAggro];
    static const float RANGE_COMPASS = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeCompass];
    static const float RANGE_SPIRIT = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeSpirit];
    static const float RANGE_EARSHOT = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeEarshot];
    static const float RANGE_CASTING = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeCasting];
    static const float RANGE_PROJECTILE = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeProjectile];
    static const float RANGE_HALF = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeHalf];
    static const float RANGE_TOUCH = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeTouch];
    static const float RANGE_ADJECENT = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeAdjecent];

    static const float RANGE_VISIBLE = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeVisible];

    // Compass radius
    if (QueryObjects(res, RANGE_COMPASS))
    {
        for (const auto& o : res)
        {
            if (o != this && o->GetType() > AB::GameProtocol::ObjectTypeSentToPlayer)
            {
                Math::Vector3 objectPos = o->GetPosition();
                Math::Vector3 myPos = GetPosition();
                if (myPos.Distance(objectPos) <= RANGE_AGGRO)
                    ranges_[Ranges::Aggro].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_COMPASS)
                    ranges_[Ranges::Compass].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_SPIRIT)
                    ranges_[Ranges::Spirit].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_EARSHOT)
                    ranges_[Ranges::Earshot].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_CASTING)
                    ranges_[Ranges::Casting].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_PROJECTILE)
                    ranges_[Ranges::Projectile].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_HALF)
                    ranges_[Ranges::HalfCompass].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_TOUCH)
                    ranges_[Ranges::Touch].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_ADJECENT)
                    ranges_[Ranges::Adjecent].push_back(o);
                if (myPos.Distance(objectPos) <= RANGE_VISIBLE)
                    ranges_[Ranges::Visible].push_back(o);
            }
        }
    }
}

void Actor::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);
    UpdateRanges();

    Skill* skill = nullptr;
    int skillIndex = -1;

    InputItem input;

    stateComp_.Update(timeElapsed);
    resourceComp_.Update(timeElapsed);

    // Multiple inputs of the same type overwrite previous
    while (inputs_.Get(input))
    {
        switch (input.type)
        {
        case InputType::Move:
        {
            if (!IsDead())
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
            }
            break;
        }
        case InputType::Turn:
        {
            if (!IsDead())
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
            }
            break;
        }
        case InputType::Direction:
        {
            if (!IsDead())
            {
                // No aurorunComp_.Reset() because manually setting the camera does not
                // stop autorunning
                if (!autorunComp_.autoRun_)
                    moveComp_.SetDirection(input.data[InputDataDirection].GetFloat());
            }
            break;
        }
        case InputType::SetState:
        {
            if (!IsDead())
            {
                AB::GameProtocol::CreatureState state =
                    static_cast<AB::GameProtocol::CreatureState>(input.data[InputDataState].GetInt());
                stateComp_.SetState(state);
            }
            break;
        }
        case InputType::Goto:
        {
            if (!IsDead())
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
            }
            break;
        }
        case InputType::Follow:
        {
            if (!IsDead())
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
        }
            break;
        }
        case InputType::Attack:
            if (!IsDead())
                stateComp_.SetState(AB::GameProtocol::CreatureStateAttacking);
            break;
        case InputType::UseSkill:
        {
            if (!IsDead())
            {
                skillIndex = static_cast<uint32_t>(input.data[InputDataSkillIndex].GetInt());
                skill = GetSkill(skillIndex);
                if (skill)
                {
                    if (auto selObj = selectedObject_.lock())
                    {
                        std::shared_ptr<Actor> target = selObj->GetThisDynamic<Actor>();
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
            if (sourceId == id_)
                source = this;
            else
                source = dynamic_cast<Actor*>(GetGame()->GetObjectById(sourceId).get());

            if (source)
            {
                if (targetId != source->GetSelectedObjectId())
                {
                    source->selectedObject_ = GetGame()->GetObjectById(targetId);
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
            if (sourceId == id_)
                source = this;
            else
                source = dynamic_cast<Actor*>(GetGame()->GetObjectById(sourceId).get());

            if (source)
            {
                auto clickedObj = GetGame()->GetObjectById(targetId);
                if (clickedObj)
                {
                    clickedObj->OnClicked(source->GetThis<Actor>());
                }
            }
            break;
        }
        case InputType::Cancel:
            if (stateComp_.GetState() == AB::GameProtocol::CreatureStateUsingSkill)
                stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
            else if (stateComp_.GetState() == AB::GameProtocol::CreatureStateAttacking)
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
        moveComp_.Update(timeElapsed);
        autorunComp_.Update(timeElapsed);

        if (moveComp_.moved_)
        {
            message.AddByte(AB::GameProtocol::GameObjectPositionChange);
            message.Add<uint32_t>(id_);
            message.Add<float>(transformation_.position_.x_);
            message.Add<float>(transformation_.position_.y_);
            message.Add<float>(transformation_.position_.z_);
            moveComp_.moved_ = false;
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
        moveComp_.turned_ = false;
        moveComp_.directionSet_ = false;
    }

    attackComp_.Update(timeElapsed);
    skillsComp_.Update(timeElapsed);
    skills_.Update(timeElapsed);
    effectsComp_.Update(timeElapsed);

    resourceComp_.Write(message);
}

bool Actor::Die()
{
    if (!IsDead())
    {
        resourceComp_.SetHealth(Components::SetValueType::Absolute, 0);
        resourceComp_.SetEnergy(Components::SetValueType::Absolute, 0);
        resourceComp_.SetAdrenaline(Components::SetValueType::Absolute, 0);
        stateComp_.SetState(AB::GameProtocol::CreatureStateDead);
        return true;
    }
    return false;
}

bool Actor::IsEnemy(Actor* other)
{
    if (GetGroupId() == other->GetGroupId())
        return false;
    // TODO: What if different groups are allies
    return true;
}

}
