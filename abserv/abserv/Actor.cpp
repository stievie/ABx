/**
 * Copyright 2017-2020 Stefan Ascher
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

#include "stdafx.h"
#include "Actor.h"
#include "AreaOfEffect.h"
#include "CollisionComp.h"
#include "ConfigManager.h"
#include "DamageComp.h"
#include "EffectManager.h"
#include "Game.h"
#include "GameManager.h"
#include "Group.h"
#include "HealComp.h"
#include "Item.h"
#include "ItemFactory.h"
#include "ItemsCache.h"
#include "OctreeQuery.h"
#include "PartyManager.h"
#include "Player.h"
#include "ProgressComp.h"
#include "ScriptManager.h"
#include "SelectionComp.h"
#include <abshared/TemplEncoder.h>
#include <AB/ProtocolCodes.h>
#include <abshared/AttribAlgos.h>
#include <abshared/Attributes.h>

namespace Game {

void Actor::RegisterLua(kaguya::State& state)
{
    state["Actor"].setClass(kaguya::UserdataMetatable<Actor, GameObject>()
        .addFunction("GetLevel", &Actor::GetLevel)
        .addFunction("GetSkillBar", &Actor::GetSkillBar)
        .addFunction("GetSelectedObject", &Actor::_LuaGetSelectedObject)
        .addFunction("SetSelectedObject", &Actor::_LuaSetSelectedObject)
        .addFunction("UseSkill", &Actor::UseSkill)
        .addFunction("Attack", &Actor::Attack)
        .addFunction("IsAttackingActor", &Actor::IsAttackingActor)
        .addFunction("GetCurrentSkill", &Actor::GetCurrentSkill)
        .addFunction("GetWeapon", &Actor::GetWeapon)
        .addFunction("IsInWeaponRange", &Actor::IsInWeaponRange)
        .addFunction("GetAttackSpeed", &Actor::GetAttackSpeed)
        .addFunction("GetAttackDamageType", &Actor::GetAttackDamageType)
        .addFunction("GetAttackDamage", &Actor::GetAttackDamage)
        .addFunction("GetCriticalChance", &Actor::GetCriticalChance)
        .addFunction("GetDamagePos", &Actor::GetDamagePos)
        .addFunction("Damage", &Actor::Damage)
        .addFunction("DrainLife", &Actor::DrainLife)
        .addFunction("DrainEnergy", &Actor::DrainEnergy)
        .addFunction("AddEnergy", &Actor::AddEnergy)
        .addFunction("SetHealthRegen", &Actor::SetHealthRegen)
        .addFunction("InterruptAttack", &Actor::InterruptAttack)
        .addFunction("InterruptSkill", &Actor::InterruptSkill)
        .addFunction("Interrupt", &Actor::Interrupt)
        .addFunction("Healing", &Actor::Healing)
        .addFunction("GetResource", &Actor::GetResource)
        .addFunction("SetResource", &Actor::SetResource)
        .addFunction("GetMorale", &Actor::GetMorale)
        .addFunction("IncreaseMorale", &Actor::IncreaseMorale)
        .addFunction("DecreaseMorale", &Actor::DecreaseMorale)
        .addFunction("GetDeaths", &Actor::GetDeaths)

        .addFunction("IsUndestroyable", &Actor::IsUndestroyable)
        .addFunction("SetUndestroyable", &Actor::SetUndestroyable)
        .addFunction("GetSpeed", &Actor::GetSpeed)
        .addFunction("SetSpeed", &Actor::SetSpeed)
        .addFunction("AddSpeed", &Actor::AddSpeed)
        .addFunction("AddEffect", &Actor::_LuaAddEffect)
        .addFunction("HasEffect", &Actor::_LuaHasEffect)
        .addFunction("IsMoving", &Actor::IsMoving)
        .addFunction("RemoveEffect", &Actor::_LuaRemoveEffect)
        .addFunction("GetLastEffect", &Actor::_LuaGetLastEffect)
        .addFunction("IsHitting", &Actor::IsHitting)
        .addFunction("IsImmobilized", &Actor::IsImmobilized)

        .addFunction("GotoPosition", &Actor::_LuaGotoPosition)
        .addFunction("FollowObject", &Actor::_LuaFollowObject)
        .addFunction("FaceObject", &Actor::FaceObject)
        .addFunction("HeadTo", &Actor::_LuaHeadTo)
        .addFunction("IsEnemy", &Actor::IsEnemy)
        .addFunction("IsAlly", &Actor::IsAlly)
        .addFunction("AddFriendFoe", &Actor::AddFriendFoe)
        .addFunction("RemoveFriendFoe", &Actor::RemoveFriendFoe)
        .addFunction("GetGroup", &Actor::GetGroup)
        .addFunction("GetGroupId", &Actor::GetGroupId)
        .addFunction("SetGroupId", &Actor::SetGroupId)

        .addFunction("SetSpawnPoint", &Actor::SetSpawnPoint)
        .addFunction("GetHomePos", &Actor::_LuaGetHomePos)
        .addFunction("SetHomePos", &Actor::_LuaSetHomePos)
        .addFunction("GotoHomePos", &Actor::GotoHomePos)
        .addFunction("IsDead", &Actor::IsDead)
        .addFunction("IsKnockedDown", &Actor::IsKnockedDown)
        .addFunction("KnockDown", &Actor::KnockDown)
        .addFunction("Die", &Actor::Die)
        .addFunction("Resurrect", &Actor::Resurrect)
        .addFunction("CancelAction", &Actor::CancelAction)

        .addFunction("DropRandomItem", &Actor::DropRandomItem)
        .addFunction("AddAOE", &Actor::_LuaAddAOE)
        .addFunction("IsUsingSkillOfType", &Actor::IsUsingSkillOfType)

        .addFunction("GetEnemiesInRange", &Actor::_LuaGetEnemiesInRange)
        .addFunction("GetEnemyCountInRange", &Actor::GetEnemyCountInRange)
        .addFunction("GetAlliesInRange", &Actor::_LuaGetAlliesInRange)
        .addFunction("GetAllyCountInRange", &Actor::GetAllyCountInRange)
        .addFunction("GetClosestEnemy", &Actor::GetClosestEnemy)
        .addFunction("GetClosestAlly", &Actor::GetClosestAlly)
        .addFunction("GetActorsInRange", &Actor::_LuaGetActorsInRange)

        .addFunction("GetAttributeRank", &Actor::GetAttributeRank)
    );
}

Actor::Actor() :
    GameObject(),
    skills_(std::make_unique<SkillBar>(*this)),
    resourceComp_(std::make_unique<Components::ResourceComp>(*this)),
    attackComp_(std::make_unique<Components::AttackComp>(*this)),
    skillsComp_(std::make_unique<Components::SkillsComp>(*this)),
    inputComp_(std::make_unique<Components::InputComp>(*this)),
    damageComp_(std::make_unique<Components::DamageComp>(*this)),
    healComp_(std::make_unique<Components::HealComp>(*this)),
    autorunComp_(std::make_unique<Components::AutoRunComp>(*this)),
    progressComp_(std::make_unique<Components::ProgressComp>(*this)),
    effectsComp_(std::make_unique<Components::EffectsComp>(*this)),
    inventoryComp_(std::make_unique<Components::InventoryComp>(*this)),
    moveComp_(std::make_unique<Components::MoveComp>(*this)),
    collisionComp_(std::make_unique<Components::CollisionComp>(*this)),    // Actor always collides
    selectionComp_(std::make_unique<Components::SelectionComp>(*this))
{
    events_.Subscribe<void(Skill*)>(EVENT_ON_ENDUSESKILL, std::bind(&Actor::OnEndUseSkill, this, std::placeholders::_1));
    events_.Subscribe<void(Skill*)>(EVENT_ON_STARTUSESKILL, std::bind(&Actor::OnStartUseSkill, this, std::placeholders::_1));

    /*
     * Default BB for humans
     * <attribute name="Size" value="0.3 1.7 0.5"/>
     * <attribute name="Offset Position" value="0 0.85 0"/>
    */
    SetCollisionShape(
        std::make_unique<Math::CollisionShape<Math::BoundingBox>>(Math::ShapeType::BoundingBox,
            CREATURTE_BB_MIN, CREATURTE_BB_MAX)
    );
    occluder_ = true;
    // By default Actors can be selected
    selectable_ = true;
}

Actor::~Actor() = default;

void Actor::Initialize()
{
    resourceComp_->UpdateResources();
    resourceComp_->SetHealth(Components::SetValueType::Absolute, resourceComp_->GetMaxHealth());
    resourceComp_->SetEnergy(Components::SetValueType::Absolute, resourceComp_->GetMaxEnergy());
    moveComp_->StoreOldPosition();
}

bool Actor::SetSpawnPoint(const std::string& group)
{
    auto game = GetGame();
    if (!game)
        return false;
    const auto sps = game->map_->GetSpawnPoints(group);
    const auto sp = game->map_->GetFreeSpawnPoint(sps);
    if (sp.Empty())
        return false;

    transformation_.position_ = sp.position;
    transformation_.position_.y_ = game->map_->GetTerrainHeight(transformation_.position_);
    transformation_.SetYRotation(sp.rotation.EulerAngles().y_);
    return true;
}

bool Actor::GotoHomePos()
{
    if (!homePos_.Equals(transformation_.position_, AT_POSITION_THRESHOLD * 2.0f))
    {
        GotoPosition(homePos_);
        return true;
    }
    // Already there
    return false;
}

bool Actor::SelectedObject(GameObject* object)
{
    if (object)
        return selectionComp_->SelectObject(object->GetId());
    return selectionComp_->SelectObject(0);
}

bool Actor::SelectedObjectById(uint32_t id)
{
    return selectionComp_->SelectObject(id);
}

void Actor::GotoPosition(const Math::Vector3& pos)
{
    Utils::VariantMap data;
    data[InputDataVertexX] = pos.x_;
    data[InputDataVertexY] = pos.y_;
    data[InputDataVertexZ] = pos.z_;
    inputComp_->Add(InputType::Goto, std::move(data));
}

bool Actor::FollowObject(GameObject* object, bool ping)
{
    if (!object)
        return false;
    if (IsImmobilized())
        return false;

    bool result = autorunComp_->Follow(object->GetPtr<GameObject>(), ping);
    if (result)
    {
        CancelAll();
        stateComp_.SetState(AB::GameProtocol::CreatureState::Moving);
        autorunComp_->SetAutoRun(true);

    }
    return result;
}

bool Actor::FollowObjectById(uint32_t objectId, bool ping)
{
    auto* target = GetGame()->GetObject<GameObject>(objectId);
    if (!target)
        return false;
    return FollowObject(target, ping);
}

bool Actor::Attack(Actor* target, bool ping)
{
    if (!target)
        return false;
    if (!CanAttack() || IsImmobilized())
        return false;

    if (!IsEnemy(target))
        return false;
    if (attackComp_->IsAttackingTarget(target))
        return true;

    // First select the target
    if (!selectionComp_->SelectObject(target->GetId()))
        return false;
    // Then attack
    return attackComp_->Attack(target->GetPtr<Actor>(), ping);
}

bool Actor::AttackById(uint32_t targetId, bool ping)
{
    auto* target = GetGame()->GetObject<Actor>(targetId);
    if (!target)
        return false;

    return Attack(target, ping);
}

bool Actor::IsAttackingActor(const Actor* target) const
{
    if (!target)
        return false;
    if (attackComp_->IsAttackingTarget(target))
        return true;
    return false;
}

bool Actor::UseSkill(int index, bool ping)
{
    if (!CanUseSkill() || IsDead())
        return false;
    if (index < 0 || index >= PLAYER_MAX_SKILLS)
        return false;

    return skillsComp_->UseSkill(index, ping) == AB::GameProtocol::SkillError::None;
}

void Actor::CancelAll()
{
    attackComp_->Cancel();
    skillsComp_->Cancel();
    autorunComp_->Reset();
    stateComp_.Reset();
}

void Actor::CancelAction()
{
    inputComp_->Add(InputType::Cancel);
}

bool Actor::AddToInventory(uint32_t itemId)
{
    auto* cache = GetSubsystem<ItemsCache>();
    Item* item = cache->Get(itemId);
    if (item)
    {
        // By default just delete the item
        auto* factory = GetSubsystem<ItemFactory>();
        factory->DeleteConcrete(item->concreteItem_.uuid);
    }
    return true;
}

void Actor::DropRandomItem()
{
    auto game = GetGame();
    if (auto killer = killedBy_.lock())
    {
        // Killed by some killer, get the party of the killer and chose a random player
        // of that party in range as drop target.
        auto* partyMngr = GetSubsystem<PartyManager>();
        auto party = partyMngr->Get(killer->GetGroupId());
        Actor* target = nullptr;
        if (party)
            target = party->GetRandomPlayerInRange(this, Ranges::HalfCompass);
        else
            target = killer.get();
        if (!target)
            // Drop nothing when no target
            return;

        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(std::bind(&Game::AddRandomItemDropFor, game, this, target))
        );
    }
    else
    {
        // Not killed by an actor, drop for any player in game
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(std::bind(&Game::AddRandomItemDrop, game, this))
        );
    }
}

void Actor::SetState(AB::GameProtocol::CreatureState state)
{
    Utils::VariantMap data;
    data[InputDataState] = static_cast<uint8_t>(state);
    inputComp_->Add(InputType::SetState, std::move(data));
}

void Actor::_LuaSetHomePos(const Math::STLVector3& pos)
{
    homePos_ = pos;
    if (Math::Equals(homePos_.y_, 0.0f))
        GetGame()->map_->UpdatePointHeight(homePos_);
}

void Actor::_LuaHeadTo(const Math::STLVector3& pos)
{
    HeadTo(pos);
}

Math::STLVector3 Actor::_LuaGetHomePos()
{
    return static_cast<Math::STLVector3>(homePos_);
}

void Actor::_LuaFollowObject(GameObject* object)
{
    if (object)
        FollowObject(object, false);
}

void Actor::_LuaAddEffect(Actor* source, uint32_t index, uint32_t time)
{
#ifdef DEBUG_GAME
    LOG_DEBUG << "Effect " << index << " added to " << GetName() << std::endl;
#endif
    effectsComp_->AddEffect(source ? source->GetPtr<Actor>() : std::shared_ptr<Actor>(), index, time);
}

bool Actor::_LuaHasEffect(uint32_t index)
{
    return effectsComp_->HasEffect(index);
}

void Actor::_LuaRemoveEffect(uint32_t index)
{
    effectsComp_->RemoveEffect(index);
}

void Actor::AddFriendFoe(uint32_t frnd, uint32_t foe)
{
    groupMask_ |= (frnd | (foe << 16));
    stateComp_.GroupMaskChanged();
}

void Actor::RemoveFriendFoe(uint32_t frnd, uint32_t foe)
{
    groupMask_ &= ~(frnd | (foe << 16));
    stateComp_.GroupMaskChanged();
}

Effect* Actor::_LuaGetLastEffect(AB::Entities::EffectCategory category)
{
    auto effect = effectsComp_->GetLast(category);
    if (effect)
        return effect.get();
    return nullptr;
}

GameObject* Actor::_LuaGetSelectedObject()
{
    return selectionComp_->GetSelectedObject();
}

void Actor::_LuaSetSelectedObject(GameObject* object)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = GetId();    // Source
    if (object)
        data[InputDataObjectId2] = object->GetId();   // Target
    else
        data[InputDataObjectId2] = 0;   // Target
    inputComp_->Add(InputType::Select, std::move(data));
}

std::vector<Actor*> Actor::_LuaGetActorsInRange(Ranges range)
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](GameObject& o)
    {
        if (o.IsPlayerOrNpcType())
        {
            auto* actor = To<Actor>(&o);
            if (actor)
                result.push_back(actor);
        }
        return Iteration::Continue;
    });
    return result;
}

std::vector<Actor*> Actor::_LuaGetEnemiesInRange(Ranges range)
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](GameObject& o)
    {
        if (o.IsPlayerOrNpcType())
        {
            auto* actor = To<Actor>(&o);
            if (actor->IsEnemy(this))
                result.push_back(actor);
        }
        return Iteration::Continue;
    });
    return result;
}

size_t Actor::GetEnemyCountInRange(Ranges range) const
{
    size_t result = 0;
    VisitInRange(range, [&](const GameObject& o)
    {
        if (o.IsPlayerOrNpcType())
        {
            const auto& actor = To<Actor>(o);
            if (actor.IsEnemy(this))
                ++result;
        }
        return Iteration::Continue;
    });
    return result;
}

std::vector<Actor*> Actor::_LuaGetAlliesInRange(Ranges range)
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](GameObject& o)
    {
        if (o.IsPlayerOrNpcType())
        {
            auto* actor = To<Actor>(&o);
            if (actor->IsAlly(this))
                result.push_back(actor);
        }
        return Iteration::Continue;
    });
    return result;
}

size_t Actor::GetAllyCountInRange(Ranges range) const
{
    // At least 1 ally that's we
    size_t result = 1;
    VisitInRange(range, [&](const GameObject& o)
    {
        if (o.IsPlayerOrNpcType())
        {
            const auto& actor = To<Actor>(o);
            if (actor.IsAlly(this))
                ++result;
        }
        return Iteration::Continue;
    });
    return result;
}

Actor* Actor::GetClosestEnemy(bool undestroyable, bool unselectable)
{
    return GetClosestActor([&](const Actor& actor) -> bool
    {
        if (!actor.IsSelectable() && !unselectable)
            return false;
        if ((actor.IsUndestroyable() || actor.IsDead()) && !undestroyable)
            return false;
        if (!IsEnemy(&actor))
            return false;
        return true;
    });
}

Actor* Actor::GetClosestAlly(bool undestroyable, bool unselectable)
{
    return GetClosestActor([&](const Actor& actor) -> bool
    {
        if (!actor.IsSelectable() && !unselectable)
            return false;
        if ((actor.IsUndestroyable() || actor.IsDead()) && !undestroyable)
            return false;
        if (!IsAlly(&actor))
            return false;
        return true;
    });
}

bool Actor::Serialize(sa::PropWriteStream& stream)
{
    using namespace AB::GameProtocol;
    uint32_t validFields = ObjectSpawnDataFieldName | ObjectSpawnDataFieldLevel |
        ObjectSpawnDataFieldSex | ObjectSpawnDataFieldProf | ObjectSpawnDataFieldModelIndex |
        ObjectSpawnDataFieldSkills;
    if (Is<Player>(*this))
    {
        validFields |= ObjectSpawnDataFieldPvpCharacter;
    }

    stream.Write<uint32_t>(validFields);

    if (!GameObject::Serialize(stream))
        return false;
    stream.Write<uint32_t>(GetLevel());
    if (Is<Player>(*this))
    {
        stream.Write<uint8_t>(To<Player>(*this).data_.pvp ? 1 : 0);
    }
    stream.Write<uint8_t>(GetSex());
    stream.Write<uint32_t>(static_cast<uint32_t>(GetProfIndex()));
    stream.Write<uint32_t>(static_cast<uint32_t>(GetProf2Index()));
    stream.Write<uint32_t>(GetItemIndex());
    const std::string skills = skills_->Encode();
    stream.WriteString(skills);
    return true;
}

void Actor::WriteSpawnData(Net::NetworkMessage& msg)
{
    GameObject::WriteSpawnData(msg);

    using namespace AB::GameProtocol;
    uint32_t validFields = ObjectSpawnFieldPos | ObjectSpawnFieldRot | ObjectSpawnFieldScale |
        ObjectSpawnFieldUndestroyable | ObjectSpawnFieldSelectable | ObjectSpawnFieldState |
        ObjectSpawnFieldSpeed | ObjectSpawnFieldGroupId | ObjectSpawnFieldGroupPos;

    if (groupMask_ != 0)
        validFields |= ObjectSpawnFieldGroupMask;

    msg.Add<uint32_t>(validFields);

    msg.Add<float>(transformation_.position_.x_);
    msg.Add<float>(transformation_.position_.y_);
    msg.Add<float>(transformation_.position_.z_);
    msg.Add<float>(transformation_.GetYRotation());
    msg.Add<float>(transformation_.scale_.x_);
    msg.Add<float>(transformation_.scale_.y_);
    msg.Add<float>(transformation_.scale_.z_);
    msg.Add<bool>(undestroyable_);
    msg.Add<bool>(selectable_);
    msg.Add<uint8_t>(static_cast<uint8_t>(stateComp_.GetState()));
    msg.Add<float>(GetSpeed());
    msg.Add<uint32_t>(GetGroupId());
    msg.Add<uint8_t>(static_cast<uint8_t>(GetGroupPos()));
    if (groupMask_ != 0)
        msg.Add<uint32_t>(groupMask_);
    sa::PropWriteStream data;
    size_t dataSize;
    Serialize(data);
    const char* cData = data.GetStream(dataSize);
    msg.Add(std::string(cData, dataSize));
    resourceComp_->Write(msg, true);
    effectsComp_->Write(msg);
}

Item* Actor::GetWeapon() const
{
    return inventoryComp_->GetWeapon();
}

void Actor::OnEndUseSkill(Skill* skill)
{
    // These change the state
    if (skill->IsChangingState())
    {
        attackComp_->Pause(false);
        if (!stateComp_.IsDead())
            stateComp_.SetState(AB::GameProtocol::CreatureState::Idle);
    }
}

void Actor::OnStartUseSkill(Skill* skill)
{
    // These change the state
    if (skill->IsChangingState())
    {
        attackComp_->Pause();
        autorunComp_->Reset();
        stateComp_.SetState(AB::GameProtocol::CreatureState::UsingSkill);
    }
}

void Actor::HeadTo(const Math::Vector3& pos)
{
    if (!IsImmobilized())
        moveComp_->HeadTo(pos);
}

void Actor::FaceObject(GameObject* object)
{
    if (object && !IsImmobilized() && object != this)
        HeadTo(object->transformation_.position_);
}

bool Actor::IsInWeaponRange(Actor* actor) const
{
    Item* weapon = GetWeapon();
    if (!weapon)
        // If there is no weapon, use feasts!
        return (GetDistance(actor) - AVERAGE_BB_EXTENDS) <= RANGE_TOUCH;
    const float range = weapon->GetWeaponRange();
    if (range == 0.0f)
        return false;
    return (GetDistance(actor) - AVERAGE_BB_EXTENDS) <= range;
}

float Actor::GetArmorEffect(DamageType damageType, DamagePos pos, float penetration)
{
    // To calculate the effect of the armor to the damage. The damage is multiplied by value
    switch (damageType)
    {
    case DamageType::Holy:
    case DamageType::Shadow:
    case DamageType::Chaos:
    case DamageType::LifeDrain:
    case DamageType::Typeless:
    case DamageType::Dark:
        // Ignoring armor -> full damage, armor no effect
        return 1.0f;
    default:
        break;
    }
    const int baseArmor = inventoryComp_->GetArmor(damageType, pos);
    int armorMod = 0;
    effectsComp_->GetArmor(damageType, armorMod);
    const float totalArmor = static_cast<float>(baseArmor) * (1.0f - penetration) + static_cast<float>(armorMod);
    return 0.5f + ((totalArmor - 60.0f) / 40.0f);
}

uint32_t Actor::GetAttackSpeed()
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return ATTACK_SPEED_STAFF;
    const uint32_t speed = weapon->GetWeaponAttackSpeed();
    uint32_t modSpeed = speed;
    effectsComp_->GetAttackSpeed(weapon, modSpeed);

    // https://wiki.guildwars.com/wiki/Attack_speed
    // Max IAS 133%, max DAS 50%
    modSpeed = Math::Clamp(modSpeed,
        static_cast<uint32_t>(static_cast<float>(speed) * MAX_DAS),
        static_cast<uint32_t>(static_cast<float>(speed) * MAX_IAS));
    return modSpeed;
}

float Actor::GetAttackSpeedIncrease(uint32_t speed)
{
    Item* weapon = GetWeapon();
    const uint32_t normalSpeed = weapon ? weapon->GetWeaponAttackSpeed() : ATTACK_SPEED_STAFF;
    return static_cast<float>(normalSpeed) / static_cast<float>(speed);
}

DamageType Actor::GetAttackDamageType()
{
    Item* weapon = GetWeapon();
    if (!weapon)
        // No weapon makes Slashing damage
        return DamageType::Slashing;
    DamageType type = DamageType::Unknown;
    weapon->GetWeaponDamageType(type);
    effectsComp_->GetAttackDamageType(type);
    return type;
}

int32_t Actor::GetAttackDamage(bool critical)
{
    Item* weapon = GetWeapon();
    if (!weapon)
    {
        // make small damage without weapon, maybe with feasts :D
        const int32_t level = static_cast<int32_t>(GetLevel());
        // Level 20 actors make 5 damage. Lower actors make least 1 damage
        return Math::Clamp(level / 4, 1, level);
    }
    int32_t damage = 0;
    // Get weapon damage with mods
    weapon->GetWeaponDamage(damage, critical);
    const Attribute attrib = weapon->GetWeaponAttribute();
    const uint32_t req = weapon->GetWeaponRequirement();
    if (GetAttributeRank(attrib) < req)
        // If requirements are not met, damage is the half
        damage /= 2;

    // Effects may modify the damage
    effectsComp_->GetAttackDamage(damage);
    return damage;
}

float Actor::GetArmorPenetration()
{
    // 1. Attribute strength
    const float strength = static_cast<float>(GetAttributeRank(Attribute::Strength));
    float value = (strength * 0.01f);
    // 2. Weapons
    value += inventoryComp_->GetArmorPenetration();
    // 3. Effects
    float ea = 0.0f;
    effectsComp_->GetArmorPenetration(ea);
    value += ea;
    return value;
}

float Actor::GetCriticalChance(const Actor* other)
{
    if (!other)
        return 0.0f;
    // https://www.guildwiki.de/wiki/Kritische_Treffer

    // From behind is always critical
    const float diff = transformation_.GetYRotation() - other->transformation_.GetYRotation();
    if (fabs(diff) < BEHIND_ANGLE)
        // If rotation is < 30 deg it's from behind -> always critical even without weapon
        return 1.0f;

    Item* weapon = GetWeapon();
    if (!weapon)
        return 0.0f;

    const Attribute attrib = weapon->GetWeaponAttribute();
    const float attribVal = static_cast<float>(GetAttributeRank(attrib));
    const float myLevel = static_cast<float>(GetLevel());
    const float otherLevel = static_cast<float>(other->GetLevel());

    const float val1 = ((8.0f * myLevel) - (15.0f * otherLevel) + (4.0f * attribVal) + (6 * std::min(attribVal, ((myLevel + 4.0f) / 2.0f))) - 100.0f) / 40.0f;
    const float val2 = (1.0f - (attribVal / 100.0f));
    const float val3 = (attribVal / 100.0f);
    return 0.5f * (2.0f * val1) * val2 + val3;
}

void Actor::SetResource(Components::ResourceType type, Components::SetValueType t, int value)
{
    resourceComp_->SetValue(type, t, value);
}

int Actor::DrainLife(Actor* source, uint32_t index, int value)
{
    return damageComp_->DrainLife(source, index, value);
}

int Actor::AddEnergy(int value)
{
    return resourceComp_->AddEnergy(value);
}

int Actor::DrainEnergy(int value)
{
    return resourceComp_->DrainEnergy(value);
}

void Actor::SetHealthRegen(int value)
{
    Components::SetValueType vt = value < 0 ?
        Components::SetValueType::Decrease :
        Components::SetValueType::Increase;
    resourceComp_->SetHealthRegen(vt, abs(value));
}

bool Actor::InterruptAttack()
{
    bool success = true;
    CallEvent<void(bool&)>(EVENT_ON_INTERRUPTING_ATTACK, success);
    if (!success)
        return false;
    return attackComp_->Interrupt();
}

bool Actor::InterruptSkill(AB::Entities::SkillType type)
{
    Skill* skill = skillsComp_->GetCurrentSkill();
    if (!skill)
        return false;
    bool success = true;
    CallEvent<void(AB::Entities::SkillType, Skill*, bool&)>(EVENT_ON_INTERRUPTING_SKILL, type, skill, success);
    if (!success)
        return false;
    return skillsComp_->Interrupt(type);
}

bool Actor::Interrupt()
{
    bool ret = InterruptAttack();
    if (ret)
        return true;
    return InterruptSkill(AB::Entities::SkillTypeSkill);
}

std::string Actor::GetClassLevel() const
{
    if (!skills_)
        return "Lvl" + std::to_string(GetLevel());
    std::string classes = skills_->GetClasses();
    if (classes.empty())
        return "Lvl" + std::to_string(GetLevel());

    classes += std::to_string(GetLevel());
    return classes;
}

uint32_t Actor::GetAttributePoints() const
{
    return GetAttribPoints(GetLevel());
}

void Actor::AdvanceLevel()
{
    resourceComp_->UpdateResources();
}

bool Actor::IsUsingSkillOfType(AB::Entities::SkillType type, int32_t minActivationTime) const
{
    const auto* skill = skills_->GetCurrentSkill();
    if (skill && skill->IsUsing() && skill->IsType(type) &&
        (skill->activation_ > minActivationTime))
        return true;
    return false;
}

Skill* Actor::GetCurrentSkill() const
{
    return skills_->GetCurrentSkill();
}

bool Actor::SetEquipment(const std::string& ciUuid)
{
    auto* factory = GetSubsystem<ItemFactory>();
    uint32_t itemId = factory->GetConcreteId(ciUuid);
    if (itemId == 0)
        return false;
    inventoryComp_->SetEquipment(itemId);
    return true;
}

bool Actor::SetInventory(const std::string& ciUuid)
{
    auto* factory = GetSubsystem<ItemFactory>();
    uint32_t itemId = factory->GetConcreteId(ciUuid);
    if (itemId == 0)
        return false;
    return inventoryComp_->SetInventoryItem(itemId, nullptr);
}

bool Actor::SetChest(const std::string& ciUuid)
{
    auto* factory = GetSubsystem<ItemFactory>();
    uint32_t itemId = factory->GetConcreteId(ciUuid);
    if (itemId == 0)
        return false;
    return inventoryComp_->SetChestItem(itemId, nullptr);
}

const std::string& Actor::GetPlayerUuid() const
{
    static const std::string empty(Utils::Uuid::EMPTY_UUID);
    return empty;
}

const std::string& Actor::GetAccountUuid() const
{
    static const std::string empty(Utils::Uuid::EMPTY_UUID);
    return empty;
}

void Actor::_LuaGotoPosition(const Math::STLVector3& pos)
{
    if (IsImmobilized())
        return;

    Math::Vector3 _pos(pos);
    if (Math::Equals(_pos.y_, 0.0f))
        GetGame()->map_->UpdatePointHeight(_pos);
    GotoPosition(_pos);
}

void Actor::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);

    // Update all
    stateComp_.Update(timeElapsed);
    resourceComp_->Update(timeElapsed);
    inputComp_->Update(timeElapsed, message);
    selectionComp_->Update(timeElapsed);

    attackComp_->Update(timeElapsed);
    skillsComp_->Update(timeElapsed);
    effectsComp_->Update(timeElapsed);
    damageComp_->Update(timeElapsed);
    healComp_->Update(timeElapsed);
    uint32_t flags = Components::MoveComp::UpdateFlagTurn;
    if (!autorunComp_->IsAutoRun())
        flags |= Components::MoveComp::UpdateFlagMove;
    moveComp_->Update(timeElapsed, flags);
    autorunComp_->Update(timeElapsed);
    // After move/autorun resolve collisions
    collisionComp_->Update(timeElapsed);
    progressComp_->Update(timeElapsed);

    if (moveComp_->moved_ && octant_)
    {
        Math::Octree* octree = octant_->GetRoot();
        octree->AddObjectUpdate(this);
    }

    // Write all
    stateComp_.Write(message);
    moveComp_->Write(message);
    selectionComp_->Write(message);

    skillsComp_->Write(message);
    attackComp_->Write(message);
    effectsComp_->Write(message);
    resourceComp_->Write(message);
    damageComp_->Write(message);
    healComp_->Write(message);
    progressComp_->Write(message);
}

bool Actor::Die()
{
    if (!IsDead())
    {
        attackComp_->Cancel();
        stateComp_.SetState(AB::GameProtocol::CreatureState::Dead);
        resourceComp_->SetHealth(Components::SetValueType::Absolute, 0);
        resourceComp_->SetEnergy(Components::SetValueType::Absolute, 0);
        resourceComp_->SetAdrenaline(Components::SetValueType::Absolute, 0);
        damageComp_->Touch();
        autorunComp_->SetAutoRun(false);
        DecreaseMorale();
        killedBy_ = damageComp_->GetLastDamager();
        CallEvent<void(Actor*, Actor*)>(EVENT_ON_DIED, this, damageComp_->GetLastDamager().get());
        return true;
    }
    return false;
}

bool Actor::Resurrect(int precentHealth, int percentEnergy)
{
    if (IsDead())
    {
        const int health = (resourceComp_->GetMaxHealth() / 100) * precentHealth;
        resourceComp_->SetHealth(Components::SetValueType::Absolute, health);
        const int energy = (resourceComp_->GetMaxEnergy() / 100) * percentEnergy;
        resourceComp_->SetEnergy(Components::SetValueType::Absolute, energy);
        damageComp_->Touch();
        stateComp_.SetState(AB::GameProtocol::CreatureState::Idle);
        CallEvent<void(int,int)>(EVENT_ON_RESURRECTED, health, energy);
        return true;
    }
    return false;
}

bool Actor::KnockDown(Actor* source, uint32_t time)
{
    if (IsDead())
        return false;
    if (IsKnockedDown())
        return false;

    if (time == 0)
        time = DEFAULT_KNOCKDOWN_TIME;
    bool ret = true;
    CallEvent<void(Actor*, uint32_t, bool&)>(EVENT_ON_KNOCKING_DOWN, source, time, ret);
    if (!ret)
        return false;

    ret = stateComp_.KnockDown(time);
    if (ret)
    {
        // KD interrupts all regardless of effects that may prevent interrupting.
        // The only way to prevent this is an effect that prevents KDs.
        attackComp_->Interrupt();
        skillsComp_->Interrupt(AB::Entities::SkillTypeSkill);
        autorunComp_->Reset();
        CallEvent<void(uint32_t)>(EVENT_ON_KNOCKED_DOWN, time);
    }
    return ret;
}

int Actor::Healing(Actor* source, uint32_t index, int value)
{
    if (IsDead())
        return 0;
    int val = value;
    CallEvent<void(Actor*, int&)>(EVENT_ON_HEALING, source, val);
    healComp_->Healing(source, index, val);
    CallEvent<void(int)>(EVENT_ON_HEALED, val);
    return val;
}

int Actor::Damage(Actor* source, uint32_t index, DamageType type, int value)
{
    if (IsDead())
        return 0;
    int val = value;
    bool crit = false;
    effectsComp_->GetDamage(type, val, crit);
    damageComp_->ApplyDamage(source, index, type, val, 0.0f, false);
    return val;
}

bool Actor::IsEnemy(const Actor* other) const
{
    if (!other)
        return false;

    if ((GetGroupId() != 0) && GetGroupId() == other->GetGroupId())
        // Same group members are always friends
        return false;
    // Return true if we have a matching bit of our foe mask in their friend mask
    return ((GetFoeMask() & other->GetFriendMask()) != 0);
}

bool Actor::IsAlly(const Actor* other) const
{
    if (!other)
        return false;

    if ((GetGroupId() != 0) && GetGroupId() == other->GetGroupId())
        // Same group members are always friends
        return true;
    // Return true if they have matching bits in the friend mask
    return ((GetFriendMask() & other->GetFriendMask()) != 0);
}

uint32_t Actor::GetAttributeRank(Attribute index)
{
    uint32_t result = 0;
    // Skilled points
    const AttributeValue* val = skills_->GetAttribute(index);
    if (val != nullptr)
        result = val->value;
    // Increase by equipment
    result += inventoryComp_->GetAttributeRank(index);

    // Effects can in- or decrease the attribute rank
    int32_t value = 0;
    effectsComp_->GetAttributeRank(index, value);
    if (value == 0)
        return result;

    if (value < 0)
    {
        // It's not possible to have a negative attribute rank
        if (static_cast<uint32_t>(abs(value)) < result)
            result -= static_cast<uint32_t>(abs(value));
        else
            result = 0;
    }
    else
        result += static_cast<uint32_t>(value);
    return result;
}

AreaOfEffect* Actor::_LuaAddAOE(const std::string& script, uint32_t index,
    const Math::STLVector3& pos)
{
    auto result = GetGame()->AddAreaOfEffect(script, GetPtr<Actor>(), index, pos);
    if (result)
        return result.get();
    return nullptr;
}

Group* Actor::GetGroup() const
{
    if (auto g = GetGame())
        return g->GetGroup(GetGroupId());
    return nullptr;
}

int Actor::GetMorale() const
{
    return resourceComp_->GetMorale();
}

bool Actor::IncreaseMorale()
{
    if (!IsUndestroyable())
        return resourceComp_->IncreaseMorale();
    return false;
}

bool Actor::DecreaseMorale()
{
    if (!IsUndestroyable())
        return resourceComp_->DecreaseMorale();
    return false;
}

unsigned Actor::GetDeaths() const
{
    return progressComp_->GetDeaths();
}

DamagePos Actor::GetDamagePos() const
{
    return damageComp_->GetDamagePos();
}

GameObject* Actor::GetSelectedObject() const
{
    return selectionComp_->GetSelectedObject();
}

uint32_t Actor::GetSelectedObjectId() const
{
    return selectionComp_->GetSelectedObjectId();
}

}
