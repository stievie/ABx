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


#include "Item.h"
#include "Actor.h"
#include "DataProvider.h"
#include "ItemFactory.h"
#include "ItemsCache.h"
#include "Script.h"
#include "ScriptManager.h"
#include "Skill.h"

namespace Game {

void Item::RegisterLua(kaguya::State& state)
{
    state["Item"].setClass(kaguya::UserdataMetatable<Item>()
        .addFunction("GetWeaponRange", &Item::GetWeaponRange)
        .addFunction("GetWeaponAttackSpeed", &Item::GetWeaponAttackSpeed)
        .addFunction("GetWeaponDamageType", &Item::GetWeaponDamageType)
        .addFunction("GetWeaponDamage", &Item::GetWeaponDamage)
        .addFunction("GetType", &Item::GetType)
        .addFunction("IsStackable", &Item::IsStackable)
    );
}

void Item::InitializeLua()
{
    Lua::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

bool Item::LoadConcrete(const AB::Entities::ConcreteItem& item)
{
    concreteItem_ = item;
    sa::PropReadStream stream;
    stream.Init(item.itemStats.data(), item.itemStats.length());
    if (!stats_.Load(stream))
        LOG_WARNING << "Error loading item stats" << std::endl;

    baseMinDamage_ = stats_.GetMinDamage();
    baseMaxDamage_ = stats_.GetMaxDamage();
    auto* itemFactory = GetSubsystem<ItemFactory>();
    if (!Utils::Uuid::IsEmpty(item.upgrade1Uuid))
        upgrades_[ItemUpgrade::Pefix] = itemFactory->GetConcreteId(item.upgrade1Uuid);
    if (!Utils::Uuid::IsEmpty(item.upgrade2Uuid))
        upgrades_[ItemUpgrade::Suffix] = itemFactory->GetConcreteId(item.upgrade2Uuid);
    if (!Utils::Uuid::IsEmpty(item.upgrade3Uuid))
        upgrades_[ItemUpgrade::Inscription] = itemFactory->GetConcreteId(item.upgrade3Uuid);

    return true;
}

bool Item::LoadScript(const std::string& fileName)
{
    if (fileName.empty())
        // An item does not need a script
        return true;

    auto script = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script)
        return false;
    if (!script->Execute(luaState_))
        return false;

    if (Lua::IsFunction(luaState_, "onUpdate"))
        sa::bits::set(functions_, FunctionUpdate);
    if (Lua::IsFunction(luaState_, "getDamage"))
        sa::bits::set(functions_, FunctionGetDamage);
    if (Lua::IsFunction(luaState_, "getDamageType"))
        sa::bits::set(functions_, FunctionGetDamageType);
    if (Lua::IsFunction(luaState_, "onEquip"))
        sa::bits::set(functions_, FunctionOnEquip);
    if (Lua::IsFunction(luaState_, "onUnequip"))
        sa::bits::set(functions_, FunctionOnUnequip);
    if (Lua::IsFunction(luaState_, "getSkillCost"))
        sa::bits::set(functions_, FunctionGetSkillCost);
    if (Lua::IsFunction(luaState_, "getSkillRecharge"))
        sa::bits::set(functions_, FunctionGetSkillRecharge);
    return true;
}

void Item::CreateInsigniaStats(uint32_t level, bool maxStats)
{
    if (Lua::IsFunction(luaState_, "getHealthStats"))
    {
        int32_t health = luaState_["getHealthStats"](level, maxStats);
        stats_.SetValue(ItemStatIndex::Health, health);
    }
}

void Item::CreateWeaponStats(uint32_t level, bool maxStats)
{
    if (Lua::IsFunction(luaState_, "getDamageStats"))
    {
        int32_t minDamage = 0;
        int32_t maxDamage = 0;
        kaguya::tie(minDamage, maxDamage) = luaState_["getDamageStats"](level, maxStats);
        stats_.SetValue(ItemStatIndex::MinDamage, minDamage);
        stats_.SetValue(ItemStatIndex::MaxDamage, maxDamage);
    }
}

void Item::CreateFocusStats(uint32_t level, bool maxStats)
{
    if (Lua::IsFunction(luaState_, "getEnergyStats"))
    {
        int32_t energy = luaState_["getEnergyStats"](level, maxStats);
        stats_.SetValue(ItemStatIndex::Energy, energy);
    }
}

void Item::CreateShieldStats(uint32_t level, bool maxStats)
{
    if (Lua::IsFunction(luaState_, "getArmorStats"))
    {
        int32_t armor = luaState_["getArmorStats"](level, maxStats);
        stats_.SetValue(ItemStatIndex::Armor, armor);
    }
}

bool Item::GenerateConcrete(AB::Entities::ConcreteItem& ci, uint32_t level, bool maxStats)
{
    concreteItem_ = ci;

    switch (data_.type)
    {
    case AB::Entities::ItemType::ModifierInsignia:
        CreateInsigniaStats(level, maxStats);
        break;
    case AB::Entities::ItemType::Axe:
    case AB::Entities::ItemType::Sword:
    case AB::Entities::ItemType::Wand:
    case AB::Entities::ItemType::Spear:
    case AB::Entities::ItemType::Hammer:
    case AB::Entities::ItemType::Flatbow:
    case AB::Entities::ItemType::Hornbow:
    case AB::Entities::ItemType::Shortbow:
    case AB::Entities::ItemType::Longbow:
    case AB::Entities::ItemType::Recurvebow:
    case AB::Entities::ItemType::Staff:
    case AB::Entities::ItemType::Daggers:
    case AB::Entities::ItemType::Scyte:
        CreateWeaponStats(level, maxStats);
        break;
    case AB::Entities::ItemType::Focus:
        CreateFocusStats(level, maxStats);
        break;
    case AB::Entities::ItemType::Shield:
        CreateShieldStats(level, maxStats);
        break;
    default:
        break;
    }

    sa::PropWriteStream stream;
    stats_.Save(stream);
    size_t ssize = 0;
    const char* s = stream.GetStream(ssize);
    concreteItem_.itemStats = std::string(s, ssize);

    baseMinDamage_ = stats_.GetMinDamage();
    baseMaxDamage_ = stats_.GetMaxDamage();
    return true;
}

void Item::Update(uint32_t timeElapsed)
{
    if (HaveFunction(FunctionUpdate))
        luaState_["onUpdate"](timeElapsed);

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->Update(timeElapsed);
    }
}

Item* Item::SetUpgrade(ItemUpgrade type, uint32_t id)
{
    auto* cache = GetSubsystem<ItemsCache>();
    Item* upgrade = cache->Get(id);
    if (upgrade)
    {

        upgrades_[type] = upgrade->id_;
        switch (type)
        {
        case ItemUpgrade::Pefix:
            concreteItem_.upgrade1Uuid = upgrade->concreteItem_.uuid;
            break;
        case ItemUpgrade::Suffix:
            concreteItem_.upgrade2Uuid = upgrade->concreteItem_.uuid;
            break;
        case ItemUpgrade::Inscription:
            concreteItem_.upgrade3Uuid = upgrade->concreteItem_.uuid;
            break;
        case ItemUpgrade::__Count:
            ASSERT_FALSE();
        }
        return upgrade;
    }
    else
    {
        RemoveUpgrade(type);
    }
    return nullptr;
}

Item* Item::GetUpgrade(ItemUpgrade type)
{
    if (upgrades_[type] != 0)
    {
        auto* cache = GetSubsystem<ItemsCache>();
        return cache->Get(upgrades_[type]);
    }
    return nullptr;
}

void Item::RemoveUpgrade(ItemUpgrade type)
{
    if (upgrades_[type] != 0)
    {
        upgrades_.erase(type);
        switch (type)
        {
        case ItemUpgrade::Pefix:
            concreteItem_.upgrade1Uuid = Utils::Uuid::EMPTY_UUID;
            break;
        case ItemUpgrade::Suffix:
            concreteItem_.upgrade2Uuid = Utils::Uuid::EMPTY_UUID;
            break;
        case ItemUpgrade::Inscription:
            concreteItem_.upgrade3Uuid = Utils::Uuid::EMPTY_UUID;
            break;
        case ItemUpgrade::__Count:
            ASSERT_FALSE();
        }
    }
}

EquipPos Item::GetEquipPos() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemType::Axe:
    case AB::Entities::ItemType::Sword:
    case AB::Entities::ItemType::Wand:
    case AB::Entities::ItemType::Spear:
        return EquipPos::WeaponLeadHand;
    case AB::Entities::ItemType::Hammer:
    case AB::Entities::ItemType::Flatbow:
    case AB::Entities::ItemType::Hornbow:
    case AB::Entities::ItemType::Shortbow:
    case AB::Entities::ItemType::Longbow:
    case AB::Entities::ItemType::Recurvebow:
    case AB::Entities::ItemType::Staff:
    case AB::Entities::ItemType::Daggers:
    case AB::Entities::ItemType::Scyte:
        return EquipPos::WeaponTwoHanded;
    case AB::Entities::ItemType::Focus:
    case AB::Entities::ItemType::Shield:
        return EquipPos::WeaponOffHand;
    case AB::Entities::ItemType::ArmorHead:
        return EquipPos::ArmorHead;
    case AB::Entities::ItemType::ArmorChest:
        return EquipPos::ArmorChest;
    case AB::Entities::ItemType::ArmorHands:
        return EquipPos::ArmorHands;
    case AB::Entities::ItemType::ArmorLegs:
        return EquipPos::ArmorFeet;
    case AB::Entities::ItemType::ArmorFeet:
        return EquipPos::ArmorFeet;
    default:
        return EquipPos::None;
    }
}

float Item::GetWeaponRange() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemType::Axe:
    case AB::Entities::ItemType::Sword:
    case AB::Entities::ItemType::Hammer:
        return RANGE_TOUCH;
    case AB::Entities::ItemType::Flatbow:
        return RANGE_FLATBOW;
    case AB::Entities::ItemType::Hornbow:
        return RANGE_HORNBOW;
    case AB::Entities::ItemType::Shortbow:
        return RANGE_SHORTBOW;
    case AB::Entities::ItemType::Longbow:
        return RANGE_LONGBOW;
    case AB::Entities::ItemType::Recurvebow:
        return RANGE_RECURVEBOW;
    case AB::Entities::ItemType::Staff:
    case AB::Entities::ItemType::Wand:
        return RANGE_PROJECTILE;
    case AB::Entities::ItemType::Daggers:
        return RANGE_TOUCH;
    case AB::Entities::ItemType::Scyte:
        return RANGE_TOUCH;
    case AB::Entities::ItemType::Spear:
        return RANGE_SPEAR;
    default:
        return 0.0f;
    }
}

uint32_t Item::GetWeaponAttackSpeed() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemType::Axe:
        return ATTACK_SPEED_AXE;
    case AB::Entities::ItemType::Sword:
        return ATTACK_SPEED_SWORD;
    case AB::Entities::ItemType::Hammer:
        return ATTACK_SPEED_HAMMER;
    case AB::Entities::ItemType::Flatbow:
        return ATTACK_SPEED_FLATBOW;
    case AB::Entities::ItemType::Hornbow:
        return ATTACK_SPEED_HORNBOW;
    case AB::Entities::ItemType::Shortbow:
        return ATTACK_SPEED_SHORTBOW;
    case AB::Entities::ItemType::Longbow:
        return ATTACK_SPEED_LONGBOW;
    case AB::Entities::ItemType::Recurvebow:
        return ATTACK_SPEED_RECURVEBOW;
    case AB::Entities::ItemType::Staff:
        return ATTACK_SPEED_STAFF;
    case AB::Entities::ItemType::Wand:
        return ATTACK_SPEED_WAND;
    case AB::Entities::ItemType::Daggers:
        return ATTACK_SPEED_DAGGERS;
    case AB::Entities::ItemType::Scyte:
        return ATTACK_SPEED_SCYTE;
    case AB::Entities::ItemType::Spear:
        return ATTACK_SPEED_SPEAR;
    default:
        return 0;
    }
}

bool Item::IsWeaponProjectile() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemType::Axe:
    case AB::Entities::ItemType::Sword:
    case AB::Entities::ItemType::Hammer:
    case AB::Entities::ItemType::Daggers:
    case AB::Entities::ItemType::Scyte:
        return false;
    case AB::Entities::ItemType::Flatbow:
    case AB::Entities::ItemType::Hornbow:
    case AB::Entities::ItemType::Shortbow:
    case AB::Entities::ItemType::Longbow:
    case AB::Entities::ItemType::Recurvebow:
    case AB::Entities::ItemType::Staff:
    case AB::Entities::ItemType::Wand:
    case AB::Entities::ItemType::Spear:
        return true;
    default:
        return false;
    }
}

void Item::GetWeaponDamageType(DamageType& value) const
{
    value = stats_.GetDamageType();
    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetWeaponDamageType(value);
    }

    if (value != DamageType::Unknown)
        return;

    // Default weapon damage type
    switch (data_.type)
    {
    case AB::Entities::ItemType::Axe:
        value = DamageType::Piercing;
        break;
    case AB::Entities::ItemType::Sword:
        value = DamageType::Slashing;
        break;
    case AB::Entities::ItemType::Hammer:
        value = DamageType::Blunt;
        break;
    case AB::Entities::ItemType::Flatbow:
    case AB::Entities::ItemType::Hornbow:
    case AB::Entities::ItemType::Shortbow:
    case AB::Entities::ItemType::Longbow:
    case AB::Entities::ItemType::Recurvebow:
        value = DamageType::Piercing;
        break;
    case AB::Entities::ItemType::Staff:
    case AB::Entities::ItemType::Wand:
        value = DamageType::Slashing;
        break;
    case AB::Entities::ItemType::Daggers:
        value = DamageType::Piercing;
        break;
    case AB::Entities::ItemType::Scyte:
        value = DamageType::Slashing;
        break;
    case AB::Entities::ItemType::Spear:
        value = DamageType::Piercing;
        break;
    default:
        value = DamageType::Unknown;
        break;
    }
}

Attribute Item::GetWeaponAttribute() const
{
    auto ret = stats_.GetAttribute();
    if (ret != Attribute::None)
        return ret;

    // Default weapon attributes
    switch (data_.type)
    {
    case AB::Entities::ItemType::Axe:
        return Attribute::AxeMatery;
    case AB::Entities::ItemType::Sword:
        return Attribute::SwordsManship;
    case AB::Entities::ItemType::Hammer:
        return Attribute::HammerMastery;
    case AB::Entities::ItemType::Flatbow:
    case AB::Entities::ItemType::Hornbow:
    case AB::Entities::ItemType::Shortbow:
    case AB::Entities::ItemType::Longbow:
    case AB::Entities::ItemType::Recurvebow:
        return Attribute::MarkMansship;
    default:
        return Attribute::None;
    }
}

uint32_t Item::GetWeaponRequirement() const
{
    return stats_.GetRequirement();
}

void Item::GetArmor(DamageType damageType, int& value) const
{
    value = stats_.GetArmor(damageType);

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetArmor(damageType, value);
    }
}

void Item::GetArmorPenetration(float& value) const
{
    switch (data_.type)
    {
    case AB::Entities::ItemType::Hornbow:
        // Hornbow adds 10% armor penetration
        value += 0.1f;
        break;
    default:
        break;
    }

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetArmorPenetration(value);
    }
}

void Item::GetResources(int& maxHealth, int& maxEnergy)
{
    maxHealth += stats_.GetHealth();
    maxEnergy += stats_.GetEnergy();

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetResources(maxHealth, maxEnergy);
    }
}

void Item::GetSkillRecharge(Skill *skill, uint32_t &recharge)
{
    if (HaveFunction(FunctionGetSkillRecharge))
    {
        recharge = luaState_["getSkillRecharge"](skill, recharge);
    }

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetSkillRecharge(skill, recharge);
    }
}

void Item::GetSkillCost(Skill* skill, int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp)
{
    if (HaveFunction(FunctionGetSkillCost))
    {
        kaguya::tie(activation, energy, adrenaline, overcast, hp) =
            luaState_["getSkillCost"](skill, activation, energy, adrenaline, overcast, hp);
    }

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetSkillCost(skill, activation, energy, adrenaline, overcast, hp);
    }
}

void Item::GetAttributeRank(Attribute index, uint32_t& value)
{
    // Equipment (e.g. runes) may increase the attributes
    value += stats_.GetAttributeIncrease(index);

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetAttributeRank(index, value);
    }
}

void Item::OnEquip(Actor* target)
{
    if (HaveFunction(FunctionOnEquip))
        luaState_["onEquip"](target);

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->OnEquip(target);
    }
}

void Item::OnUnequip(Actor* target)
{
    if (HaveFunction(FunctionOnUnequip))
        luaState_["onUnequip"](target);

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->OnUnequip(target);
    }
}

uint32_t Item::GetValue() const
{
    uint32_t result = concreteItem_.value;
    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            result += item->concreteItem_.value;
    }
    return result;
}

AB::Entities::ItemType Item::GetType() const
{
    return data_.type;
}

void Item::GetWeaponDamage(int32_t& value, bool critical)
{
    if (HaveFunction(FunctionGetDamage))
    {
        float val = luaState_["getDamage"](baseMinDamage_, baseMaxDamage_, critical);
        value = static_cast<int32_t>(val);
    }

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetWeaponDamage(value, critical);
    }
}

}
