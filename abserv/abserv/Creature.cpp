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
    creatureState_(CreatureStateIdle),
    moveDir_(AB::GameProtocol::MoveDirectionNone),
    selectedObject_(nullptr)
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

    InputItem input;
    CreatureState newState = creatureState_;

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
                newState = CreatureStateMoving;
            }
            else
            {
                if (creatureState_ == CreatureStateMoving)
                    newState = CreatureStateIdle;
            }
            break;
        }
        case InputTypeTurn:
        {
            turnDir_ = static_cast<AB::GameProtocol::TurnDirection>(input.data[InputDataDirection].GetInt());
            if (turnDir_ > AB::GameProtocol::TurnDirectionNone)
            {
                newState = CreatureStateMoving;
            }
            else
            {
                if (creatureState_ == CreatureStateMoving)
                    newState = CreatureStateIdle;
            }
            break;
        }
        case InputTypeAttack:
            newState = CreatureStateAttacking;
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
                    newState = CreatureStateUsingSkill;
            }
            break;
        }
        case InputTypeSelect:
        {
            uint32_t targetId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            selectedObject_ = GetGame()->GetObjectById(targetId);
            message.AddByte(AB::GameProtocol::GameObjectSelectTarget);
            message.Add<uint32_t>(id_);
            if (selectedObject_)
                message.Add<uint32_t>(selectedObject_->id_);
            else
                // Clear Target
                message.Add<uint32_t>(0);
            break;
        }
        case InputTypeCancelSkill:
            break;
        case InputTypeCancelAttack:
            break;
        case InputTypeNone:
            break;
        }
    }

    if (newState != creatureState_)
    {
        creatureState_ = newState;
#ifdef DEBUG_PROTOCOL
//        LOG_DEBUG << "New State " << static_cast<int>(creatureState_) << std::endl;
#endif
    }

    switch (creatureState_)
    {
    case CreatureStateIdle:
        break;
    case CreatureStateMoving:
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

        bool turned = false;
        if ((turnDir_ & AB::GameProtocol::TurnDirectionLeft) == AB::GameProtocol::TurnDirectionLeft)
        {
            Turn(((float)(timeElapsed) / 2000.0f) * speed, Math::Vector3::Down);
            turned = true;
        }
        if ((turnDir_ & AB::GameProtocol::TurnDirectionRight) == AB::GameProtocol::TurnDirectionRight)
        {
            Turn(((float)(timeElapsed) / 2000.0f) * speed, Math::Vector3::UnitY);
            turned = true;
        }
        if (turned)
        {
            message.AddByte(AB::GameProtocol::GameObjectRotationChange);
            message.Add<uint32_t>(id_);
            Math::Vector4 rot = transformation_.rotation_.AxisAngle();
            message.Add<float>(rot.w_);
        }
        break;
    }
    case CreatureStateUsingSkill:
        break;
    case CreatureStateAttacking:
        break;
    case CreatureStateEmote:
        break;
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

void Creature::Move(float speed, const Math::Vector3& amount)
{
    // new position = position + direction * speed (where speed = amount * speed)

    // It's as easy as:
    // 1. Create a matrix from the rotation,
    // 2. multiply this matrix with the moving vector and
    // 3. add the resulting vector to the current position
#ifdef HAVE_DIRECTX_MATH
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationQuaternion(transformation_.rotation_);
    Math::Vector3 a = amount * speed;
    DirectX::XMVECTOR v = DirectX::XMVector3Transform(a, m);
    transformation_.position_.x_ += v.m128_f32[0];
    transformation_.position_.y_ += v.m128_f32[1];
    transformation_.position_.z_ += v.m128_f32[2];
#else
    Matrix4 m = Matrix4::CreateFromQuaternion(transformation_.rotation_);
    Vector3 a = amount * speed;
    Vector3 v = m * a;
    transformation_.position_ += v;
#endif
}

void Creature::Turn(float angle, const Math::Vector3& axis)
{
#ifdef HAVE_DIRECTX_MATH
    transformation_.rotation_ = DirectX::XMQuaternionNormalize(
        DirectX::XMQuaternionMultiply(transformation_.rotation_,
        DirectX::XMQuaternionRotationAxis(axis, angle)));
#else
    Math::Quaternion delta = Math::Quaternion::FromAxisAngle(axis, angle);
    // Multiply current rotation by delta rotation
    transformation_.rotation_ *= delta;
    transformation_.rotation_.Normalize();
#endif
}

}
