#pragma once

#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include "Script.h"
#include "Damage.h"
#include "ItemStats.h"
#include "Attributes.h"

namespace Game {

class Actor;

enum class ItemUpgrade
{
    Pefix = 0,           // Insignia for armor, prefix for weapon
    Suffix,              // Rune for armor, suffix for weapon
    Inscription          // For lead/off hand weapons
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
    };
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    uint32_t functions_{ FunctionNone };
    std::map<ItemUpgrade, std::unique_ptr<Item>> upgrades_;
    int32_t baseMinDamage_{ 0 };
    int32_t baseMaxDamage_{ 0 };
    ItemStats stats_;
    void InitializeLua();
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
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
    Item* SetUpgrade(ItemUpgrade type, std::unique_ptr<Item> upgrade);
    Item* GetUpgrade(ItemUpgrade type);
    void RemoveUpgrade(ItemUpgrade type);
    EquipPos GetEquipPos() const;
    float GetWeaponRange() const;
    uint32_t GetWeaponAttackSpeed() const;
    /// Return true when the weapon is throwing projectiles
    bool IsWeaponProjectile() const;
    void GetWeaponDamageType(DamageType& value) const;
    void GetWeaponDamage(int32_t& value, bool critical);
    AttributeIndices GetWeaponAttribute() const;
    uint32_t GetWeaponRequirement() const;
    void GetArmor(DamageType damageType, int& value) const;
    void GetArmorPenetration(float& value) const;
    void GetResources(int& maxHealth, int& maxEnergy);
    void GetAttributeValue(uint32_t index, uint32_t& value);
    void OnEquip(Actor* target);
    void OnUnequip(Actor* target);
    /// Get value of this item with all mods
    uint32_t GetValue() const
    {
        uint32_t result = concreteItem_.value;
        for (const auto& u : upgrades_)
        {
            if (u.second)
                result += u.second->concreteItem_.value;
        }
        return result;
    }

    AB::Entities::ItemType GetType() const;
    bool IsStackAble() const;
    bool IsArmor() const
    {
        return data_.type == AB::Entities::ItemTypeArmorHead ||
            data_.type == AB::Entities::ItemTypeArmorChest ||
            data_.type == AB::Entities::ItemTypeArmorHands ||
            data_.type == AB::Entities::ItemTypeArmorLegs ||
            data_.type == AB::Entities::ItemTypeArmorFeet;
    }

    AB::Entities::Item data_;
    AB::Entities::ConcreteItem concreteItem_;
};

}
