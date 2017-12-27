#include "stdafx.h"
#include "Creature.h"
#include "EffectManager.h"
#include "Game.h"
#include <AB/ProtocolCodes.h>
#include "Logger.h"

#include "DebugNew.h"

namespace Game {

void Creature::RegisterLua(kaguya::State& state)
{
    state["Creature"].setClass(kaguya::UserdataMetatable<Creature, GameObject>()
        .addFunction("GetLevel", &Creature::GetLevel)

        .addFunction("GetSpeed", &Creature::GetSpeed)
        .addFunction("SetSpeed", &Creature::SetSpeed)
        .addFunction("GetEnergy", &Creature::GetEnergy)
        .addFunction("SetEnergy", &Creature::SetEnergy)
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
    creatureState_(AB::GameProtocol::CreatureStateIdle),
    moveDir_(AB::GameProtocol::MoveDirectionNone),
    turnDir_(AB::GameProtocol::TurnDirectionNone)
{
}

void Creature::AddEffect(uint32_t id, uint32_t ticks)
{
    RemoveEffect(id);

    auto effect = EffectManager::Instance.Get(id);
    if (effect)
    {
        effects_.push_back(std::move(effect));
        effect->Start(GetThis<Creature>(), ticks);
    }
}

void Creature::AddEffectByName(const std::string& name, uint32_t ticks)
{
    uint32_t id = EffectManager::Instance.GetEffectId(name);
    AddEffect(id, ticks);
}

void Creature::DeleteEffect(uint32_t id)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [&](std::unique_ptr<Effect> const& current)
    {
        return current->id_ == id;
    });
    if (it != effects_.end())
    {
        effects_.erase(it);
    }
}

void Creature::RemoveEffect(uint32_t id)
{
    auto it = std::find_if(effects_.begin(), effects_.end(), [&](std::unique_ptr<Effect> const& current)
    {
        return current->id_ == id;
    });
    if (it != effects_.end())
    {
        (*it)->Remove();
        DeleteEffect((*it)->id_);
    }
}

void Creature::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);

    Skill* skill = nullptr;
    bool turned = false;
    bool directionSet = false;
    bool newDirection = false;
    float worldAngle = 0.0f;

    InputItem input;
    AB::GameProtocol::CreatureState newState = creatureState_;

    // Multiple inputs of the same type overwrite previous
    while (inputs_.Get(input))
    {
        switch (input.type)
        {
        case InputTypeMove:
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
        case InputTypeTurn:
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
        case InputTypeDirection:
        {
            worldAngle = input.data[InputDataDirection].GetFloat();
            newDirection = true;
            break;
        }
        case InputTypeAttack:
            newState = AB::GameProtocol::CreatureStateAttacking;
            break;
        case InputTypeUseSkill:
        {
            uint32_t skillIndex = static_cast<uint32_t>(input.data[InputDataSkillIndex].GetInt());
            skill = GetSkill(skillIndex);
            if (skill)
            {
                // These do not change the state
                if (!skill->IsType(SkillTypeStance) && !skill->IsType(SkillTypeFlashEnchantment)
                    && !skill->IsType(SkillTypeShout))
                    newState = AB::GameProtocol::CreatureStateUsingSkill;
            }
            break;
        }
        case InputTypeSelect:
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
        case InputTypeCancelSkill:
            if (creatureState_ == AB::GameProtocol::CreatureStateUsingSkill)
                newState = AB::GameProtocol::CreatureStateIdle;
            break;
        case InputTypeCancelAttack:
            if (creatureState_ == AB::GameProtocol::CreatureStateAttacking)
                newState = AB::GameProtocol::CreatureStateIdle;
            break;
        }
    }

    if (newState != creatureState_)
    {
//        LOG_DEBUG << "New state " << (int)newState << std::endl;
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
            Move(((float)(timeElapsed) / 100.0f) * speed, Math::Vector3::UnitZ);
            moved = true;
        }
        if ((moveDir_ & AB::GameProtocol::MoveDirectionSouth) == AB::GameProtocol::MoveDirectionSouth)
        {
            // Move slower backward
            Move(((float)(timeElapsed) / 100.0f) * speed, Math::Vector3::Back / 2.0f);
            moved = true;
        }
        if ((moveDir_ & AB::GameProtocol::MoveDirectionWest) == AB::GameProtocol::MoveDirectionWest)
        {
            Move(((float)(timeElapsed) / 100.0f) * speed, Math::Vector3::Left / 2.0f);
            moved = true;
        }
        if ((moveDir_ & AB::GameProtocol::MoveDirectionEast) == AB::GameProtocol::MoveDirectionEast)
        {
            Move(((float)(timeElapsed) / 100.0f) * speed, Math::Vector3::UnitX / 2.0f);
            moved = true;
        }
        if (moved)
        {
            message.AddByte(AB::GameProtocol::GameObjectPositionChange);
            message.Add<uint32_t>(id_);
            message.AddVector3(transformation_.position_);
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
            DeleteEffect(effect->id_);
            continue;
        }
        effect->Update(timeElapsed);
    }
}

bool Creature::Serialize(IO::PropWriteStream& stream)
{
    if (!GameObject::Serialize(stream))
        return false;
    return true;
}

void Creature::Move(float speed, const Math::Vector3& amount)
{
    // new position = position + direction * speed (where speed = amount * speed)

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
