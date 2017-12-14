#include "stdafx.h"
#include "Creature.h"
#include "EffectManager.h"

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
    creatureState_(CreatureStateIdle)
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
    uint32_t targetId = 0;
    MoveDirection dir = MoveDirectionNorth;

    InputItem input;
    // Multiple inputs of the same type overwrite previous
    while (inputs_.Get(input))
    {
        switch (input.type)
        {
        case InputTypeMove:
            dir = static_cast<MoveDirection>(input.data[InputDataDirection].GetInt());
            creatureState_ = CreatureStateMoving;
            break;
        case InputTypeAttack:
            creatureState_ = CreatureStateAttacking;
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
                    creatureState_ = CreatureStateUsingSkill;
            }
            break;
        }
        case InputTypeSelect:
            targetId = static_cast<uint32_t>(input.data[InputDataObjectId].GetInt());
            break;
        case InputTypeCancelSkill:
            break;
        case InputTypeCancelAttack:
            break;
        case InputTypeNone:
            break;
        }
    }

    switch (creatureState_)
    {
    case CreatureStateIdle:
        break;
    case CreatureStateMoving:
        break;
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
#endif
    /*
    Matrix4 m = Matrix4::CreateFromQuaternion(rotation_);
    Vector3 a = amount * speed;
    Vector3 v = m * a;
    position_ += v;
    */
}

void Creature::Turn(float angle, const Math::Vector3& axis)
{
    Math::Quaternion delta = Math::Quaternion::FromAxisAngle(axis, angle);
    // Multiply current rotation by delta rotation
    transformation_.rotation_ *= delta;
    transformation_.rotation_.Normalize();
}

}
