#include "stdafx.h"
#include "Creature.h"
#include "EffectManager.h"
#include "Game.h"
#include <AB/ProtocolCodes.h>
#include "Logger.h"
#include "OctreeQuery.h"

#include "DebugNew.h"

namespace Game {

void Creature::RegisterLua(kaguya::State& state)
{
    state["Creature"].setClass(kaguya::UserdataMetatable<Creature, GameObject>()
        .addFunction("GetLevel", &Creature::GetLevel)
        .addFunction("GetSkillBar", &Creature::GetSkillBar)

        .addFunction("GetSpeed", &Creature::GetSpeed)
        .addFunction("SetSpeed", &Creature::SetSpeed)
        .addFunction("GetEnergy", &Creature::GetEnergy)
        .addFunction("SetEnergy", &Creature::SetEnergy)
        .addFunction("AddEffect", &Creature::AddEffect)
        .addFunction("RemoveEffect", &Creature::RemoveEffect)
        /*        .addProperty("Energy", &Creature::GetEnergy, &Creature::SetEnergy)
        .addProperty("Health", &Creature::GetHealth, &Creature::SetHealth)
        .addProperty("Adrenaline", &Creature::GetAdrenaline, &Creature::SetAdrenaline)
        .addProperty("Overcast", &Creature::GetOvercast, &Creature::SetOvercast)
        .addProperty("Skills", &Creature::GetSkill)
        .addFunction("AddEffect", &Creature::AddEffectByName)*/
    );
}

Creature::Creature() :
    GameObject(),
    skills_(this),
    creatureState_(AB::GameProtocol::CreatureStateIdle),
    lastStateChange_(Utils::AbTick()),
    moveDir_(AB::GameProtocol::MoveDirectionNone),
    turnDir_(AB::GameProtocol::TurnDirectionNone)
{
    // Creature always collides
    SetCollisionShape(
        std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(Math::ShapeTypeBoundingBox, -0.5f, 0.5f)
    );
    occluder_ = true;
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
    bool turned = false;
    bool directionSet = false;
    bool newDirection = false;
    float worldAngle = 0.0f;

    InputItem input;
    AB::GameProtocol::CreatureState newState = creatureState_;

    if (newState == AB::GameProtocol::CreatureStateEmoteCry &&
        lastStateChange_ + 10000 < Utils::AbTick())
    {
        // Reset some emotes after 10 seconds
        newState = AB::GameProtocol::CreatureStateIdle;
    }

    // Multiple inputs of the same type overwrite previous
    while (inputs_.Get(input))
    {
        switch (input.type)
        {
        case InputType::Move:
        {
            moveDir_ = static_cast<AB::GameProtocol::MoveDirection>(input.data[InputDataDirection].GetInt());
            if (moveDir_ > AB::GameProtocol::MoveDirectionNone)
            {
                newState = AB::GameProtocol::CreatureStateMoving;
            }
            else
            {
                // Reset to Idle when neither moving nor turning
                if (creatureState_ == AB::GameProtocol::CreatureStateMoving &&
                    turnDir_ == AB::GameProtocol::TurnDirectionNone)
                    newState = AB::GameProtocol::CreatureStateIdle;
            }
            break;
        }
        case InputType::Turn:
        {
            turnDir_ = static_cast<AB::GameProtocol::TurnDirection>(input.data[InputDataDirection].GetInt());
            if (turnDir_ > AB::GameProtocol::TurnDirectionNone)
            {
                newState = AB::GameProtocol::CreatureStateMoving;
            }
            else
            {
                if (creatureState_ == AB::GameProtocol::CreatureStateMoving &&
                    moveDir_ == AB::GameProtocol::MoveDirectionNone)
                    newState = AB::GameProtocol::CreatureStateIdle;
            }
            break;
        }
        case InputType::Direction:
        {
            worldAngle = input.data[InputDataDirection].GetFloat();
            newDirection = true;
            break;
        }
        case InputType::Goto:
        {
            // TODO: Just adjust move direction to next points
            const Math::Vector3 dest = {
                input.data[InputDataVertexX].GetFloat(),
                input.data[InputDataVertexY].GetFloat(),
                input.data[InputDataVertexZ].GetFloat()
            };
            GetGame()->map_->navMesh_->FindPath(wayPoints_, transformation_.position_,
                dest);
            if (wayPoints_.size() != 0)
            {

            }
            break;
        }
        case InputType::Follow:
        {
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            followedObject_ = GetGame()->GetObjectById(targetId);
            break;
        }
        case InputType::Attack:
            newState = AB::GameProtocol::CreatureStateAttacking;
            break;
        case InputType::UseSkill:
        {
            skillIndex = static_cast<uint32_t>(input.data[InputDataSkillIndex].GetInt());
            skill = GetSkill(skillIndex);
            if (skill)
            {
                std::shared_ptr<Creature> target = std::dynamic_pointer_cast<Creature>(selectedObject_.lock());
                if (target)
                {
                    // Can use skills only on Creatures not all GameObjects
                    skills_.UseSkill(skillIndex, target);
                    // These do not change the state
                    if (skill->IsChangingState())
                        newState = AB::GameProtocol::CreatureStateUsingSkill;
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
                source->selectedObject_ = GetGame()->GetObjectById(targetId);
                message.AddByte(AB::GameProtocol::GameObjectSelectTarget);
                message.Add<uint32_t>(source->id_);
                if (auto sel = source->selectedObject_.lock())
                    message.Add<uint32_t>(sel->id_);
                else
                    // Clear Target
                    message.Add<uint32_t>(0);
            }
            break;
        }
        case InputType::CancelSkill:
            if (creatureState_ == AB::GameProtocol::CreatureStateUsingSkill)
                newState = AB::GameProtocol::CreatureStateIdle;
            break;
        case InputType::CancelAttack:
            if (creatureState_ == AB::GameProtocol::CreatureStateAttacking)
                newState = AB::GameProtocol::CreatureStateIdle;
            break;
        case InputType::Command:
        {
            AB::GameProtocol::CommandTypes type = static_cast<AB::GameProtocol::CommandTypes>(input.data[InputDataCommandType].GetInt());
            const std::string& cmd = input.data[InputDataCommandData].GetString();
            HandleCommand(type, cmd, message, newState);
            break;
        }
        }
    }

    if (newState != creatureState_)
    {
#ifdef DEBUG_GAME
        LOG_DEBUG << "New state " << (int)newState << std::endl;
#endif
        lastStateChange_ = Utils::AbTick();
        creatureState_ = newState;
        message.AddByte(AB::GameProtocol::GameObjectStateChange);
        message.Add<uint32_t>(id_);
        message.AddByte(creatureState_);
    }

    switch (creatureState_)
    {
    case AB::GameProtocol::CreatureStateIdle:
        break;
    case AB::GameProtocol::CreatureStateMoving:
    {
        bool moved = false;
        float speed = GetActualMoveSpeed();
        if ((moveDir_ & AB::GameProtocol::MoveDirectionNorth) == AB::GameProtocol::MoveDirectionNorth)
        {
            moved |= Move(((float)(timeElapsed) / BaseSpeed) * speed, Math::Vector3::UnitZ);
        }
        if ((moveDir_ & AB::GameProtocol::MoveDirectionSouth) == AB::GameProtocol::MoveDirectionSouth)
        {
            // Move slower backward
            moved |= Move(((float)(timeElapsed) / BaseSpeed) * speed, Math::Vector3::Back / 2.0f);
        }
        if ((moveDir_ & AB::GameProtocol::MoveDirectionWest) == AB::GameProtocol::MoveDirectionWest)
        {
            moved |= Move(((float)(timeElapsed) / BaseSpeed) * speed, Math::Vector3::Left / 2.0f);
        }
        if ((moveDir_ & AB::GameProtocol::MoveDirectionEast) == AB::GameProtocol::MoveDirectionEast)
        {
            moved |= Move(((float)(timeElapsed) / BaseSpeed) * speed, Math::Vector3::UnitX / 2.0f);
        }
        if (moved)
        {
            message.AddByte(AB::GameProtocol::GameObjectPositionChange);
            message.Add<uint32_t>(id_);
            message.Add<float>(transformation_.position_.x_);
            message.Add<float>(transformation_.position_.y_);
            message.Add<float>(transformation_.position_.z_);
        }

        if (!newDirection)
        {
            if ((turnDir_ & AB::GameProtocol::TurnDirectionLeft) == AB::GameProtocol::TurnDirectionLeft)
            {
                Turn(((float)(timeElapsed) / 2000.0f) * speed);
                turned = true;
            }
            if ((turnDir_ & AB::GameProtocol::TurnDirectionRight) == AB::GameProtocol::TurnDirectionRight)
            {
                Turn(-((float)(timeElapsed) / 2000.0f) * speed);
                turned = true;
            }
        }
        else
        {
            if (transformation_.rotation_ != worldAngle)
            {
                SetDirection(worldAngle);
                directionSet = true;
            }
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
    if (turned || directionSet)
    {
        message.AddByte(AB::GameProtocol::GameObjectRotationChange);
        message.Add<uint32_t>(id_);
        message.Add<float>(transformation_.rotation_);
        message.Add<uint8_t>(directionSet ? 1 : 0);
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

bool Creature::Serialize(IO::PropWriteStream& stream)
{
    if (!GameObject::Serialize(stream))
        return false;
    stream.Write<uint8_t>(GetSex());
    stream.Write<uint32_t>(GetModelIndex());
    return true;
}

void Creature::DoCollisions()
{
    std::vector<GameObject*> c;
    Math::BoundingBox box = GetWorldBoundingBox();
    if (QueryObjects(c, box))
    {
        for (auto& ci : c)
        {
            if (ci != this && ((collisionMask_ & ci->collisionMask_) == ci->collisionMask_))
            {
                Math::Vector3 move;
                if (Collides(ci, move))
                {
                    transformation_.position_ += move;
                }
            }
        }
    }

    // Keep on ground
    float y = GetGame()->map_->terrain_->GetHeight(transformation_.position_);
    transformation_.position_.y_ = y;
}

bool Creature::Move(float speed, const Math::Vector3& amount)
{
    // new position = position + direction * speed (where speed = amount * speed)

    Math::Vector3 oldPos = transformation_.position_;

    // It's as easy as:
    // 1. Create a matrix from the rotation,
    // 2. multiply this matrix with the moving vector and
    // 3. add the resulting vector to the current position
#ifdef HAVE_DIRECTX_MATH
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationAxis(Math::Vector3::UnitY, -transformation_.rotation_);
    Math::Vector3 a = amount * speed;
    DirectX::XMVECTOR v = DirectX::XMVector3Transform(a, m);
    transformation_.position_.x_ += v.m128_f32[0];
    transformation_.position_.y_ += v.m128_f32[1];
    transformation_.position_.z_ += v.m128_f32[2];
#else
    Matrix4 m = Matrix4::CreateFromQuaternion(transformation_.GetQuaternion());
    Vector3 a = amount * speed;
    Vector3 v = m * a;
    transformation_.position_ += v;
#endif

    DoCollisions();
    bool moved = oldPos != transformation_.position_;

    if (moved && octant_)
    {
        Math::Octree* octree = octant_->GetRoot();
        octree->AddObjectUpdate(this);
    }
    return moved;
}

void Creature::Turn(float angle)
{
    transformation_.rotation_ += angle;
    // Angle should be >= 0 and < 2 * PI
    if (transformation_.rotation_ >= 2.0f * (float)M_PI)
        transformation_.rotation_ -= 2.0f * (float)M_PI;
    else if (transformation_.rotation_ < 0.0f)
        transformation_.rotation_ += 2.0f * (float)M_PI;
}

void Creature::SetDirection(float worldAngle)
{
    transformation_.rotation_ = worldAngle;
    // Angle should be >= 0 and < 2 * PI
    if (transformation_.rotation_ >= 2.0f * (float)M_PI)
        transformation_.rotation_ -= 2.0f * (float)M_PI;
    else if (transformation_.rotation_ < 0.0f)
        transformation_.rotation_ += 2.0f * (float)M_PI;
}

}
