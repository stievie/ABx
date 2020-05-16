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

#pragma once

#include <memory>
#include <kaguya/kaguya.hpp>
#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include "Script.h"
#include <abshared/Damage.h>
#include "ItemStats.h"
#include <abshared/Attributes.h>
#include <sa/Bits.h>

namespace Game {

class Actor;
class Skill;

enum class ItemUpgrade
{
    Pefix = 0,           // Insignia for armor, prefix for weapon
    Suffix,              // Rune for armor, suffix for weapon
    Inscription,         // For lead/off hand weapons
    __Count
};

enum class EquipPos : uint32_t
{
    None = 0,
    ArmorHead = 1,
    ArmorChest,
    ArmorHands,
    ArmorLegs,
    ArmorFeet,

    WeaponLeadHand,
    WeaponOffHand,
    WeaponTwoHanded
};

typedef std::map<ItemUpgrade, uint32_t> UpgradesMap;
typedef std::map<EquipPos, uint32_t> EquipmentMap;

class Item
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionGetDamage = 1 << 1,
        FunctionGetDamageType = 1 << 2,
        FunctionOnEquip = 1 << 3,
        FunctionOnUnequip = 1 << 4,
        FunctionGetSkillCost = 1 << 5,
        FunctionGetSkillRecharge = 1 << 6,
    };
    kaguya::State luaState_;
    ea::shared_ptr<Script> script_;
    uint32_t functions_{ FunctionNone };
    UpgradesMap upgrades_;
    int32_t baseMinDamage_{ 0 };
    int32_t baseMaxDamage_{ 0 };
    ItemStats stats_;
    void InitializeLua();
    bool HaveFunction(Function func) const
    {
        return sa::bits::is_set(functions_, func);
    }
    void CreateInsigniaStats(uint32_t level, bool maxStats);
    void CreateWeaponStats(uint32_t level, bool maxStats);
    void CreateFocusStats(uint32_t level, bool maxStats);
    void CreateShieldStats(uint32_t level, bool maxStats);
public:
    static void RegisterLua(kaguya::State& state);

    Item() = delete;
    explicit Item(const AB::Entities::Item& item) :
        data_(item)
    {
        InitializeLua();
    }
    // non-copyable
    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;

    ~Item() = default;

    bool LoadConcrete(const AB::Entities::ConcreteItem& item);
    bool LoadScript(const std::string& fileName);
    bool GenerateConcrete(AB::Entities::ConcreteItem& ci, uint32_t level, bool maxStats);
    void Update(uint32_t timeElapsed);
    /// Upgrade this item
    Item* SetUpgrade(ItemUpgrade type, uint32_t id);
    Item* GetUpgrade(ItemUpgrade type);
    void RemoveUpgrade(ItemUpgrade type);
    EquipPos GetEquipPos() const;
    float GetWeaponRange() const;
    uint32_t GetWeaponAttackSpeed() const;
    /// Return true when the weapon is throwing projectiles
    bool IsWeaponProjectile() const;
    void GetWeaponDamageType(DamageType& value) const;
    void GetWeaponDamage(int32_t& value, bool critical);
    Attribute GetWeaponAttribute() const;
    uint32_t GetWeaponRequirement() const;
    void GetArmor(DamageType damageType, int& value) const;
    void GetArmorPenetration(float& value) const;
    void GetResources(int& maxHealth, int& maxEnergy);
    void GetSkillCost(Skill* skill, int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
    void GetSkillRecharge(Skill* skill, uint32_t& recharge);
    void GetAttributeRank(Attribute index, uint32_t& value);
    void OnEquip(Actor* target);
    void OnUnequip(Actor* target);
    /// Get value of this item with all mods
    uint32_t GetValue() const;

    AB::Entities::ItemType GetType() const;
    bool IsStackable() const { return AB::Entities::IsItemStackable(data_.itemFlags); }
    bool IsTradeable() const { return AB::Entities::IsItemTradeable(data_.itemFlags); }
    bool IsArmor() const
    {
        return AB::Entities::IsArmorItem(data_.type);
    }
    bool IsWeapon() const
    {
        return AB::Entities::IsWeaponItem(data_.type);
    }

    uint32_t id_{ std::numeric_limits<uint32_t>::min() };
    AB::Entities::Item data_;
    AB::Entities::ConcreteItem concreteItem_;
};

}
