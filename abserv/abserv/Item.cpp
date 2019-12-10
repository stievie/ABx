#include "stdafx.h"
#include "Item.h"
#include "DataProvider.h"
#include "ScriptManager.h"
#include "ItemFactory.h"
#include "Subsystems.h"
#include "Actor.h"
#include "Skill.h"
#include "ItemsCache.h"
#include "UuidUtils.h"

namespace Game {

void Item::RegisterLua(kaguya::State& state)
{
    state["Item"].setClass(kaguya::UserdataMetatable<Item>()
        .addFunction("GetWeaponRange", &Item::GetWeaponRange)
        .addFunction("GetWeaponAttackSpeed", &Item::GetWeaponAttackSpeed)
        .addFunction("GetWeaponDamageType", &Item::GetWeaponDamageType)
        .addFunction("GetWeaponDamage", &Item::GetWeaponDamage)
        .addFunction("GetType", &Item::GetType)
        .addFunction("IsStackAble", &Item::IsStackAble)
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
    IO::PropReadStream stream;
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

    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;

    if (Lua::IsFunction(luaState_, "onUpdate"))
        functions_ |= FunctionUpdate;
    if (Lua::IsFunction(luaState_, "getDamage"))
        functions_ |= FunctionGetDamage;
    if (Lua::IsFunction(luaState_, "getDamageType"))
        functions_ |= FunctionGetDamageType;
    if (Lua::IsFunction(luaState_, "onEquip"))
        functions_ |= FunctionOnEquip;
    if (Lua::IsFunction(luaState_, "onUnequip"))
        functions_ |= FunctionOnUnequip;
    if (Lua::IsFunction(luaState_, "getSkillCost"))
        functions_ |= FunctionGetSkillCost;
    if (Lua::IsFunction(luaState_, "getSkillRecharge"))
        functions_ |= FunctionGetSkillRecharge;
    return true;
}

void Item::CreateInsigniaStats(uint32_t level, bool maxStats)
{
    if (Lua::IsFunction(luaState_, "getHealthStats"))
    {
        int32_t health = luaState_["getHealthStats"](level, maxStats);
        stats_.SetValue(Stat::Health, health);
    }
}

void Item::CreateWeaponStats(uint32_t level, bool maxStats)
{
    if (Lua::IsFunction(luaState_, "getDamageStats"))
    {
        int32_t minDamage = 0;
        int32_t maxDamage = 0;
        kaguya::tie(minDamage, maxDamage) = luaState_["getDamageStats"](level, maxStats);
        stats_.SetValue(Stat::MinDamage, minDamage);
        stats_.SetValue(Stat::MaxDamage, maxDamage);
    }
}

void Item::CreateFocusStats(uint32_t level, bool maxStats)
{
    if (Lua::IsFunction(luaState_, "getEnergyStats"))
    {
        int32_t energy = luaState_["getEnergyStats"](level, maxStats);
        stats_.SetValue(Stat::Energy, energy);
    }
}

void Item::CreateShieldStats(uint32_t level, bool maxStats)
{
    if (Lua::IsFunction(luaState_, "getArmorStats"))
    {
        int32_t armor = luaState_["getArmorStats"](level, maxStats);
        stats_.SetValue(Stat::Armor, armor);
    }
}

bool Item::GenerateConcrete(AB::Entities::ConcreteItem& ci, uint32_t level, bool maxStats)
{
    concreteItem_ = ci;

    switch (data_.type)
    {
    case AB::Entities::ItemTypeModifierInsignia:
        CreateInsigniaStats(level, maxStats);
        break;
    case AB::Entities::ItemTypeAxe:
    case AB::Entities::ItemTypeSword:
    case AB::Entities::ItemTypeWand:
    case AB::Entities::ItemTypeSpear:
    case AB::Entities::ItemTypeHammer:
    case AB::Entities::ItemTypeFlatbow:
    case AB::Entities::ItemTypeHornbow:
    case AB::Entities::ItemTypeShortbow:
    case AB::Entities::ItemTypeLongbow:
    case AB::Entities::ItemTypeRecurvebow:
    case AB::Entities::ItemTypeStaff:
    case AB::Entities::ItemTypeDaggers:
    case AB::Entities::ItemTypeScyte:
        CreateWeaponStats(level, maxStats);
        break;
    case AB::Entities::ItemTypeFocus:
        CreateFocusStats(level, maxStats);
        break;
    case AB::Entities::ItemTypeShield:
        CreateShieldStats(level, maxStats);
        break;
    default:
        break;
    }

    IO::PropWriteStream stream;
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
        }
    }
}

EquipPos Item::GetEquipPos() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemTypeAxe:
    case AB::Entities::ItemTypeSword:
    case AB::Entities::ItemTypeWand:
    case AB::Entities::ItemTypeSpear:
        return EquipPos::WeaponLeadHand;
    case AB::Entities::ItemTypeHammer:
    case AB::Entities::ItemTypeFlatbow:
    case AB::Entities::ItemTypeHornbow:
    case AB::Entities::ItemTypeShortbow:
    case AB::Entities::ItemTypeLongbow:
    case AB::Entities::ItemTypeRecurvebow:
    case AB::Entities::ItemTypeStaff:
    case AB::Entities::ItemTypeDaggers:
    case AB::Entities::ItemTypeScyte:
        return EquipPos::WeaponTwoHanded;
    case AB::Entities::ItemTypeFocus:
    case AB::Entities::ItemTypeShield:
        return EquipPos::WeaponOffHand;
    case AB::Entities::ItemTypeArmorHead:
        return EquipPos::ArmorHead;
    case AB::Entities::ItemTypeArmorChest:
        return EquipPos::ArmorChest;
    case AB::Entities::ItemTypeArmorHands:
        return EquipPos::ArmorHands;
    case AB::Entities::ItemTypeArmorLegs:
        return EquipPos::ArmorFeet;
    case AB::Entities::ItemTypeArmorFeet:
        return EquipPos::ArmorFeet;
    default:
        return EquipPos::None;
    }
}

float Item::GetWeaponRange() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemTypeAxe:
    case AB::Entities::ItemTypeSword:
    case AB::Entities::ItemTypeHammer:
        return RANGE_TOUCH;
    case AB::Entities::ItemTypeFlatbow:
        return RANGE_FLATBOW;
    case AB::Entities::ItemTypeHornbow:
        return RANGE_HORNBOW;
    case AB::Entities::ItemTypeShortbow:
        return RANGE_SHORTBOW;
    case AB::Entities::ItemTypeLongbow:
        return RANGE_LONGBOW;
    case AB::Entities::ItemTypeRecurvebow:
        return RANGE_RECURVEBOW;
    case AB::Entities::ItemTypeStaff:
    case AB::Entities::ItemTypeWand:
        return RANGE_PROJECTILE;
    case AB::Entities::ItemTypeDaggers:
        return RANGE_TOUCH;
    case AB::Entities::ItemTypeScyte:
        return RANGE_TOUCH;
    case AB::Entities::ItemTypeSpear:
        return RANGE_SPEAR;
    default:
        return 0.0f;
    }
}

uint32_t Item::GetWeaponAttackSpeed() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemTypeAxe:
        return ATTACK_SPEED_AXE;
    case AB::Entities::ItemTypeSword:
        return ATTACK_SPEED_SWORD;
    case AB::Entities::ItemTypeHammer:
        return ATTACK_SPEED_HAMMER;
    case AB::Entities::ItemTypeFlatbow:
        return ATTACK_SPEED_FLATBOW;
    case AB::Entities::ItemTypeHornbow:
        return ATTACK_SPEED_HORNBOW;
    case AB::Entities::ItemTypeShortbow:
        return ATTACK_SPEED_SHORTBOW;
    case AB::Entities::ItemTypeLongbow:
        return ATTACK_SPEED_LONGBOW;
    case AB::Entities::ItemTypeRecurvebow:
        return ATTACK_SPEED_RECURVEBOW;
    case AB::Entities::ItemTypeStaff:
        return ATTACK_SPEED_STAFF;
    case AB::Entities::ItemTypeWand:
        return ATTACK_SPEED_WAND;
    case AB::Entities::ItemTypeDaggers:
        return ATTACK_SPEED_DAGGERS;
    case AB::Entities::ItemTypeScyte:
        return ATTACK_SPEED_SCYTE;
    case AB::Entities::ItemTypeSpear:
        return ATTACK_SPEED_SPEAR;
    default:
        return 0;
    }
}

bool Item::IsWeaponProjectile() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemTypeAxe:
    case AB::Entities::ItemTypeSword:
    case AB::Entities::ItemTypeHammer:
    case AB::Entities::ItemTypeDaggers:
    case AB::Entities::ItemTypeScyte:
        return false;
    case AB::Entities::ItemTypeFlatbow:
    case AB::Entities::ItemTypeHornbow:
    case AB::Entities::ItemTypeShortbow:
    case AB::Entities::ItemTypeLongbow:
    case AB::Entities::ItemTypeRecurvebow:
    case AB::Entities::ItemTypeStaff:
    case AB::Entities::ItemTypeWand:
    case AB::Entities::ItemTypeSpear:
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
    case AB::Entities::ItemTypeAxe:
        value = DamageType::Piercing;
        break;
    case AB::Entities::ItemTypeSword:
        value = DamageType::Slashing;
        break;
    case AB::Entities::ItemTypeHammer:
        value = DamageType::Blunt;
        break;
    case AB::Entities::ItemTypeFlatbow:
    case AB::Entities::ItemTypeHornbow:
    case AB::Entities::ItemTypeShortbow:
    case AB::Entities::ItemTypeLongbow:
    case AB::Entities::ItemTypeRecurvebow:
        value = DamageType::Piercing;
        break;
    case AB::Entities::ItemTypeStaff:
    case AB::Entities::ItemTypeWand:
        value = DamageType::Slashing;
        break;
    case AB::Entities::ItemTypeDaggers:
        value = DamageType::Piercing;
        break;
    case AB::Entities::ItemTypeScyte:
        value = DamageType::Slashing;
        break;
    case AB::Entities::ItemTypeSpear:
        value = DamageType::Piercing;
        break;
    default:
        value = DamageType::Unknown;
        break;
    }
}

AttributeIndices Item::GetWeaponAttribute() const
{
    auto ret = stats_.GetAttribute();
    if (ret != AttributeIndices::None)
        return ret;

    // Default weapon attributes
    switch (data_.type)
    {
    case AB::Entities::ItemTypeAxe:
        return AttributeIndices::AxeMatery;
    case AB::Entities::ItemTypeSword:
        return AttributeIndices::SwordsManship;
    case AB::Entities::ItemTypeHammer:
        return AttributeIndices::HammerMastery;
    case AB::Entities::ItemTypeFlatbow:
    case AB::Entities::ItemTypeHornbow:
    case AB::Entities::ItemTypeShortbow:
    case AB::Entities::ItemTypeLongbow:
    case AB::Entities::ItemTypeRecurvebow:
        return AttributeIndices::MarkMansship;
    default:
        return AttributeIndices::None;
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
    case AB::Entities::ItemTypeHornbow:
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

void Item::GetAttributeValue(uint32_t index, uint32_t& value)
{
    // Equipment (e.g. runes) may increase the attributes
    value += stats_.GetAttributeIncrease(index);

    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : upgrades_)
    {
        auto* item = cache->Get(i.second);
        if (item)
            item->GetAttributeValue(index, value);
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

bool Item::IsStackAble() const
{
    return data_.stackAble;
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
