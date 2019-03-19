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
#include "TemplateEncoder.h"
#include "Mechanic.h"
#include "Item.h"

#include "DebugNew.h"

namespace Game {

void Actor::RegisterLua(kaguya::State& state)
{
    state["Actor"].setClass(kaguya::UserdataMetatable<Actor, GameObject>()
        .addFunction("GetLevel", &Actor::GetLevel)
        .addFunction("GetSkillBar", &Actor::GetSkillBar)
        .addFunction("GetSelectedObject", &Actor::_LuaGetSelectedObject)
        .addFunction("SetSelectedObject", &Actor::_LuaSetSelectedObject)
        .addFunction("UseSkill", &Actor::UseSkill)
        .addFunction("GetCurrentSkill", &Actor::GetCurrentSkill)
        .addFunction("GetWeapon", &Actor::GetWeapon)
        .addFunction("IsInWeaponRange", &Actor::IsInWeaponRange)
        .addFunction("GetAttackSpeed", &Actor::GetAttackSpeed)
        .addFunction("GetAttackDamageType", &Actor::GetAttackDamageType)
        .addFunction("GetAttackDamage", &Actor::GetAttackDamage)
        .addFunction("ApplyDamage", &Actor::ApplyDamage)
        .addFunction("DrainLife", &Actor::DrainLife)
        .addFunction("DrainEnergy", &Actor::DrainEnergy)
        .addFunction("InterruptAttack", &Actor::InterruptAttack)
        .addFunction("InterruptSkill", &Actor::InterruptSkill)
        .addFunction("Interrupt", &Actor::Interrupt)
        .addFunction("Healing", &Actor::Healing)

        .addFunction("IsUndestroyable", &Actor::IsUndestroyable)
        .addFunction("SetUndestroyable", &Actor::SetUndestroyable)
        .addFunction("GetSpeed", &Actor::GetSpeed)
        .addFunction("SetSpeed", &Actor::SetSpeed)
        .addFunction("AddSpeed", &Actor::AddSpeed)
        .addFunction("AddEffect", &Actor::_LuaAddEffect)
        .addFunction("IsMoving", &Actor::IsMoving)
        .addFunction("RemoveEffect", &Actor::_LuaRemoveEffect)
        .addFunction("GetLastEffect", &Actor::_LuaGetLastEffect)
        .addFunction("IsAttacking", &Actor::IsAttacking)
        .addFunction("IsImmobilized", &Actor::IsImmobilized)

        .addFunction("GotoPosition", &Actor::_LuaGotoPosition)
        .addFunction("FollowObject", &Actor::_LuaFollowObject)
        .addFunction("GetState", &Actor::_LuaGetState)
        .addFunction("SetState", &Actor::_LuaSetState)
        .addFunction("FaceObject", &Actor::FaceObject)
        .addFunction("HeadTo", &Actor::_LuaHeadTo)
        .addFunction("IsEnemy", &Actor::IsEnemy)

        .addFunction("SetSpawnPoint", &Actor::SetSpawnPoint)
        .addFunction("GetHomePos", &Actor::_LuaGetHomePos)
        .addFunction("SetHomePos", &Actor::_LuaSetHomePos)
        .addFunction("GotoHomePos", &Actor::GotoHomePos)
        .addFunction("IsDead", &Actor::IsDead)
        .addFunction("IsKnockedDown", &Actor::IsKnockedDown)
        .addFunction("KnockDown", &Actor::KnockDown)
        .addFunction("Die", &Actor::Die)
        .addFunction("Resurrect", &Actor::Resurrect)
        .addFunction("GetActorsInRange", &Actor::GetActorsInRange)
        .addFunction("GetEnemiesInRange", &Actor::GetEnemiesInRange)
        .addFunction("GetEnemyCountInRange", &Actor::GetEnemyCountInRange)
        .addFunction("GetAlliesInRange", &Actor::GetAlliesInRange)
        .addFunction("GetAllyCountInRange", &Actor::GetAllyCountInRange)
        .addFunction("CancelAction", &Actor::CancelAction)

        .addFunction("GetAttributeValue", &Actor::GetAttributeValue)
    );
}

Actor::Actor() :
    GameObject(),
    skills_(std::make_unique<SkillBar>(*this)),
    moveComp_(*this),
    autorunComp_(*this),
    collisionComp_(*this),
    resourceComp_(*this),
    attackComp_(*this),
    effectsComp_(*this),
    equipComp_(*this),
    skillsComp_(*this),
    inputComp_(*this),
    damageComp_(*this),
    healComp_(*this),
    progressComp_(*this),
    undestroyable_(false)
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

bool Actor::SetSpawnPoint(const std::string& group)
{
    auto game = GetGame();
    if (!game)
        return false;
    auto sps = game->map_->GetSpawnPoints(group);
    auto sp = game->map_->GetFreeSpawnPoint(sps);
    if (sp.Empty())
        return false;

    transformation_.position_ = sp.position;
    transformation_.position_.y_ = game->map_->GetTerrainHeight(transformation_.position_);
    transformation_.SetYRotation(sp.rotation.EulerAngles().y_);
    return true;
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
    inputComp_.Add(InputType::Select, data);
}

void Actor::GotoPosition(const Math::Vector3& pos)
{
    Utils::VariantMap data;
    data[InputDataVertexX] = pos.x_;
    data[InputDataVertexY] = pos.y_;
    data[InputDataVertexZ] = pos.z_;
    inputComp_.Add(InputType::Goto, data);
}

void Actor::FollowObject(std::shared_ptr<GameObject> object)
{
    if (object)
        FollowObject(object->id_);
    else
        FollowObject(0);
}

void Actor::FollowObject(uint32_t objectId)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = objectId;
    inputComp_.Add(InputType::Follow, data);
}

void Actor::UseSkill(uint32_t index)
{
    Utils::VariantMap data;
    data[InputDataSkillIndex] = static_cast<uint8_t>(index);
    inputComp_.Add(InputType::UseSkill, data);
}

void Actor::CancelAction()
{
    inputComp_.Add(InputType::Cancel);
}

void Actor::_LuaSetState(int state)
{
    Utils::VariantMap data;
    data[InputDataState] = static_cast<uint8_t>(state);
    inputComp_.Add(InputType::SetState, data);
}

void Actor::_LuaSetHomePos(float x, float y, float z)
{
    homePos_ = { x, y, z };
    if (Math::Equals(homePos_.y_, 0.0f))
    {
        homePos_.y_ = GetGame()->map_->GetTerrainHeight(homePos_);
    }
}

void Actor::_LuaHeadTo(float x, float y, float z)
{
    HeadTo(Math::Vector3(x, y, z));
}

std::vector<float> Actor::_LuaGetHomePos()
{
    std::vector<float> result;
    result.push_back(homePos_.x_);
    result.push_back(homePos_.y_);
    result.push_back(homePos_.z_);
    return result;
}

void Actor::_LuaFollowObject(GameObject* object)
{
    if (object)
        FollowObject(object->id_);
    else
        FollowObject(0);
}

std::vector<Actor*> Actor::GetActorsInRange(Ranges range)
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](const std::shared_ptr<GameObject>& o)
    {
        AB::GameProtocol::GameObjectType t = o->GetType();
        if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
            result.push_back(dynamic_cast<Actor*>(o.get()));
    });
    return result;
}

std::vector<Actor*> Actor::GetEnemiesInRange(Ranges range)
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](const std::shared_ptr<GameObject>& o)
    {
        AB::GameProtocol::GameObjectType t = o->GetType();
        if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
        {
            auto actor = dynamic_cast<Actor*>(o.get());
            if (actor && actor->IsEnemy(this))
                result.push_back(actor);
        }
    });
    return result;
}

size_t Actor::GetEnemyCountInRange(Ranges range)
{
    size_t result = 0;
    VisitInRange(range, [&](const std::shared_ptr<GameObject>& o)
    {
        AB::GameProtocol::GameObjectType t = o->GetType();
        if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
        {
            auto actor = dynamic_cast<Actor*>(o.get());
            if (actor && actor->IsEnemy(this))
                ++result;
        }
    });
    return result;
}

std::vector<Actor*> Actor::GetAlliesInRange(Ranges range)
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](const std::shared_ptr<GameObject>& o)
    {
        AB::GameProtocol::GameObjectType t = o->GetType();
        if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
        {
            auto actor = dynamic_cast<Actor*>(o.get());
            if (actor && !actor->IsEnemy(this))
                result.push_back(actor);
    }
    });
    return result;
}

size_t Actor::GetAllyCountInRange(Ranges range)
{
    size_t result = 0;
    VisitInRange(range, [&](const std::shared_ptr<GameObject>& o)
    {
        AB::GameProtocol::GameObjectType t = o->GetType();
        if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
        {
            auto actor = dynamic_cast<Actor*>(o.get());
            if (actor && !actor->IsEnemy(this))
                ++result;
    }
    });
    return result;
}

void Actor::_LuaAddEffect(Actor* source, uint32_t index)
{
#ifdef DEBUG_GAME
    LOG_DEBUG << "Effect " << index << " added to " << GetName() << std::endl;
#endif
    effectsComp_.AddEffect(source ? source->GetThisDynamic<Actor>() : std::shared_ptr<Actor>(), index);
}

void Actor::_LuaRemoveEffect(uint32_t index)
{
    effectsComp_.RemoveEffect(index);
}

Effect* Actor::_LuaGetLastEffect(AB::Entities::EffectCategory category)
{
    auto effect = effectsComp_.GetLast(category);
    if (effect)
        return effect.get();
    return nullptr;
}

GameObject* Actor::_LuaGetSelectedObject()
{
    if (auto o = selectedObject_.lock())
        return o.get();
    return nullptr;
}

void Actor::_LuaSetSelectedObject(GameObject* object)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = GetId();    // Source
    if (object)
        data[InputDataObjectId2] = object->GetId();   // Target
    else
        data[InputDataObjectId2] = 0;   // Target
    inputComp_.Add(InputType::Select, data);
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
    const std::string skills = IO::TemplateEncoder::Encode(*skills_);
    stream.WriteString(skills);
    return true;
}

void Actor::WriteSpawnData(Net::NetworkMessage& msg)
{
    msg.Add<uint32_t>(id_);
    msg.Add<float>(transformation_.position_.x_);
    msg.Add<float>(transformation_.position_.y_);
    msg.Add<float>(transformation_.position_.z_);
    msg.Add<float>(transformation_.GetYRotation());
    msg.Add<float>(transformation_.scale_.x_);
    msg.Add<float>(transformation_.scale_.y_);
    msg.Add<float>(transformation_.scale_.z_);
    msg.Add<uint8_t>(undestroyable_ ? 1 : 0);
    msg.Add<uint8_t>(stateComp_.GetState());
    msg.Add<float>(GetSpeed());
    msg.Add<uint32_t>(GetGroupId());
    msg.Add<uint8_t>(static_cast<uint8_t>(GetGroupPos()));
    IO::PropWriteStream data;
    size_t dataSize;
    Serialize(data);
    const char* cData = data.GetStream(dataSize);
    msg.AddString(std::string(cData, dataSize));
    resourceComp_.Write(msg, true);
    effectsComp_.Write(msg);
}

Item* Actor::GetWeapon() const
{
    return equipComp_.GetWeapon();
}

void Actor::OnEndUseSkill(Skill* skill)
{
    // These change the state
    if (skill->IsChangingState() && !stateComp_.IsDead())
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
}

void Actor::OnStartUseSkill(Skill* skill)
{
    // These change the state
    if (skill->IsChangingState())
    {
        autorunComp_.Reset();
        stateComp_.SetState(AB::GameProtocol::CreatureStateUsingSkill);
    }
}

void Actor::HeadTo(const Math::Vector3& pos)
{
    if (!IsImmobilized())
        moveComp_.HeadTo(pos);
}

void Actor::FaceObject(GameObject* object)
{
    if (object && !IsImmobilized())
        HeadTo(object->transformation_.position_);
}

bool Actor::IsInWeaponRange(Actor* actor) const
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return false;
    const float range = weapon->GetWeaponRange();
    if (range == 0.0f)
        return false;
    return GetDistance(actor) <= range;
}

uint32_t Actor::GetAttackSpeed()
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return 0;
    const uint32_t speed = weapon->GetWeaponAttackSpeed();
    uint32_t modSpeed = speed;
    effectsComp_.GetAttackSpeed(weapon, modSpeed);

    // https://wiki.guildwars.com/wiki/Attack_speed
    // Max IAS 133%, max DAS 50%
    modSpeed = Math::Clamp(modSpeed,
        static_cast<uint32_t>(static_cast<float>(speed) * MAX_DAS),
        static_cast<uint32_t>(static_cast<float>(speed) * MAX_IAS));
    return modSpeed;
}

DamageType Actor::GetAttackDamageType()
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return DamageType::Unknown;
    DamageType type = DamageType::Unknown;
    weapon->GetWeaponDamageType(type);
    effectsComp_.GetAttackDamageType(type);
    return type;
}

int32_t Actor::GetAttackDamage(bool critical)
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return 0;
    int32_t damage = 0;
    // Get weapon damage with mods
    weapon->GetWeaponDamage(damage, critical);
    uint32_t attrib = static_cast<uint32_t>(weapon->GetWeaponAttribute());
    auto req = weapon->GetWeaponRequirement();
    if (GetAttributeValue(attrib) < req)
        // If requirements are not met, damage is the half
        damage /= 2;

    // Effects may modify the damage
    effectsComp_.GetAttackDamage(damage);
    return damage;
}

float Actor::GetAttackCriticalChance(Actor* other)
{
    if (!other)
        return 0.0f;
    // https://www.guildwiki.de/wiki/Kritische_Treffer
    Item* weapon = GetWeapon();
    if (!weapon)
        return 0.0f;

    AttributeIndices attrib = weapon->GetWeaponAttribute();
    float attribVal = static_cast<float>(GetAttributeValue(static_cast<uint32_t>(attrib)));
    float myLevel = static_cast<float>(GetLevel());
    float otherLevel = static_cast<float>(other->GetLevel());

    float val1 = ((8.0f * myLevel) - (15.0f * otherLevel) + (4.0f * attribVal) + (6 * std::min(attribVal, ((myLevel + 4.0f) / 2.0f))) - 100.0f) / 40.0f;
    float val2 = (1.0f - (attribVal / 100.0f));
    float val3 = (attribVal / 100.0f);
    return 0.5f * (2.0f * val1) * val2 + val3;
}

bool Actor::OnAttack(Actor* target)
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return false;
    bool result = true;
    effectsComp_.OnAttack(target, result);
    return result;
}

bool Actor::OnAttacked(Actor* source, DamageType type, int32_t damage)
{
    bool result = true;
    effectsComp_.OnAttacked(source, type, damage, result);
    return result;
}

bool Actor::OnGettingAttacked(Actor* source)
{
    bool result = true;
    effectsComp_.OnGettingAttacked(source, result);
    return result;
}

bool Actor::OnUseSkill(Actor* target, Skill* skill)
{
    bool result = true;
    effectsComp_.OnUseSkill(target, skill, result);
    return result;
}

bool Actor::OnSkillTargeted(Actor* source, Skill* skill)
{
    bool result = true;
    effectsComp_.OnSkillTargeted(source, skill, result);
    return result;
}

bool Actor::OnGetCriticalHit(Actor* source)
{
    bool result = true;
    effectsComp_.OnGetCriticalHit(source, result);
    return result;
}

void Actor::ApplyDamage(Actor* source, Skill* skill, DamageType type, int value)
{
    damageComp_.ApplyDamage(source, skill, type, value);
}

int Actor::DrainLife(Actor* source, Skill* skill, int value)
{
    return damageComp_.DrainLife(source, skill, value);
}

int Actor::DrainEnergy(int value)
{
    return resourceComp_.DrainEnergy(value);
}

bool Actor::OnInterruptingAttack()
{
    bool result = true;
    effectsComp_.OnInterruptingAttack(result);
    return result;
}

bool Actor::OnInterruptingSkill(AB::Entities::SkillType type, Skill* skill)
{
    bool result = true;
    effectsComp_.OnInterruptingSkill(type, skill, result);
    return result;
}

bool Actor::InterruptAttack()
{
    if (!OnInterruptingAttack())
        return false;
    return attackComp_.Interrupt();
}

bool Actor::InterruptSkill(AB::Entities::SkillType type)
{
    Skill* skill = skillsComp_.GetCurrentSkill();
    if (!skill)
        return false;
    if (!OnInterruptingSkill(type, skill))
        return false;
    return skillsComp_.Interrupt(type);
}

bool Actor::Interrupt()
{
    bool ret = InterruptAttack();
    if (ret)
        return true;
    return InterruptSkill(AB::Entities::SkillTypeSkill);
}

void Actor::OnDied()
{
    progressComp_.Died();
}

Skill* Actor::GetCurrentSkill() const
{
    return skills_->GetCurrentSkill();
}

void Actor::_LuaGotoPosition(float x, float y, float z)
{
    if (IsImmobilized())
        return;

    Math::Vector3 pos(x, y, z);
    if (Math::Equals(y, 0.0f))
        pos.y_ = GetGame()->map_->GetTerrainHeight(pos);
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

    // Compass radius
    if (QueryObjects(res, RANGE_COMPASS))
    {
        for (const auto& o : res)
        {
            if (o != this && o->GetType() > AB::GameProtocol::ObjectTypeSentToPlayer)
            {
                auto so = o->shared_from_this();
                const Math::Vector3 objectPos = o->GetPosition();
                const Math::Vector3 myPos = GetPosition();
                const float dist = myPos.Distance(objectPos);
                if (dist <= RANGE_AGGRO)
                    ranges_[Ranges::Aggro].push_back(so);
                if (dist <= RANGE_COMPASS)
                    ranges_[Ranges::Compass].push_back(so);
                if (dist <= RANGE_SPIRIT)
                    ranges_[Ranges::Spirit].push_back(so);
                if (dist <= RANGE_EARSHOT)
                    ranges_[Ranges::Earshot].push_back(so);
                if (dist <= RANGE_CASTING)
                    ranges_[Ranges::Casting].push_back(so);
                if (dist <= RANGE_PROJECTILE)
                    ranges_[Ranges::Projectile].push_back(so);
                if (dist <= RANGE_HALF_COMPASS)
                    ranges_[Ranges::HalfCompass].push_back(so);
                if (dist <= RANGE_TOUCH)
                    ranges_[Ranges::Touch].push_back(so);
                if (dist <= RANGE_ADJECENT)
                    ranges_[Ranges::Adjecent].push_back(so);
                if (dist <= RANGE_VISIBLE)
                    ranges_[Ranges::Visible].push_back(so);
            }
        }
    }
}

void Actor::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);
    UpdateRanges();

    // Update all
    stateComp_.Update(timeElapsed);
    resourceComp_.Update(timeElapsed);
    inputComp_.Update(timeElapsed, message);

    attackComp_.Update(timeElapsed);
    skillsComp_.Update(timeElapsed);
    effectsComp_.Update(timeElapsed);
    damageComp_.Update(timeElapsed);
    healComp_.Update(timeElapsed);
    moveComp_.Update(timeElapsed);
    autorunComp_.Update(timeElapsed);
    progressComp_.Update(timeElapsed);

    // Write all
    stateComp_.Write(message);
    moveComp_.Write(message);

    skillsComp_.Write(message);
    attackComp_.Write(message);
    effectsComp_.Write(message);
    resourceComp_.Write(message);
    damageComp_.Write(message);
    healComp_.Write(message);
    progressComp_.Write(message);
}

bool Actor::Die()
{
    if (!IsDead())
    {
        stateComp_.SetState(AB::GameProtocol::CreatureStateDead);
        resourceComp_.SetHealth(Components::SetValueType::Absolute, 0);
        resourceComp_.SetEnergy(Components::SetValueType::Absolute, 0);
        resourceComp_.SetAdrenaline(Components::SetValueType::Absolute, 0);
        damageComp_.Touch();
        autorunComp_.autoRun_ = false;
        OnDied();
        return true;
    }
    return false;
}

bool Actor::Resurrect(int precentHealth, int percentEnergy)
{
    if (IsDead())
    {
        int health = (resourceComp_.GetMaxHealth() / 100) * precentHealth;
        resourceComp_.SetHealth(Components::SetValueType::Absolute, health);
        int energy = (resourceComp_.GetMaxEnergy() / 100) * percentEnergy;
        resourceComp_.SetEnergy(Components::SetValueType::Absolute, energy);
        damageComp_.Touch();
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
        OnResurrected(health, energy);
        return true;
    }
    return false;
}

bool Actor::KnockDown(Actor* source, uint32_t time)
{
    if (IsDead())
        return false;

    bool ret = true;
    effectsComp_.OnKnockingDown(source, time, ret);
    if (!ret)
        return false;

    ret = stateComp_.KnockDown(time);
    if (ret)
    {
        // KD interrupts all regardless of effects that may prevent interrupting.
        // The only way to prevent this is an effect that prevents KDs.
        attackComp_.Interrupt();
        skillsComp_.Interrupt(AB::Entities::SkillTypeSkill);
        autorunComp_.Reset();
        OnKnockedDown(time);
    }
    return ret;
}

int Actor::Healing(Actor* source, Skill* skill, int value)
{
    if (IsDead())
        return 0;
    int val = value;
    effectsComp_.OnHealing(source, val);
    healComp_.Healing(source, skill, val);
    OnHealed(val);
    return val;
}

bool Actor::IsEnemy(Actor* other)
{
    if (GetGroupId() == other->GetGroupId())
        return false;
    // TODO: What if different groups are allies
    return true;
}

}
