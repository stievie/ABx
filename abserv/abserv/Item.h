#pragma once

#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include "Script.h"
#include "Damage.h"
#include "ItemStats.h"
#include "Attributes.h"

namespace Game {

enum class ItemUpgrade
{
    Pefix = 0,           // Insignia for armor, prefix for weapon
    Suffix,              // Rune for armor, suffix for weapon
    Inscription          // For lead/off hand weapons
};

class Item
{
private:
    enum Function : uint32_t
    {
        FunctionNone = 0,
        FunctionUpdate = 1,
        FunctionGetDamage = 1 << 1,
        FunctionGetDamageType = 1 << 2
    };
    kaguya::State luaState_;
    std::shared_ptr<Script> script_;
    uint32_t functions_;
    std::map<ItemUpgrade, std::unique_ptr<Item>> upgrades_;
    int32_t baseMinDamage_;
    int32_t baseMaxDamage_;
    ItemStats stats_;
    AB::Entities::ConcreteItem concreteItem_;
    void InitializeLua();
    bool HaveFunction(Function func) const
    {
        return (functions_ & func) == func;
    }
public:
    static void RegisterLua(kaguya::State& state);

    Item() = delete;
    explicit Item(const AB::Entities::Item& item) :
        functions_(FunctionNone),
        baseMinDamage_(0),
        baseMaxDamage_(0),
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
    void Update(uint32_t timeElapsed);
    /// Upgrade this item
    void SetUpgrade(ItemUpgrade type, uint32_t index);
    void RemoveUpgrade(ItemUpgrade type);
    float GetWeaponRange() const;
    uint32_t GetWeaponAttackSpeed() const;
    void GetWeaponDamageType(DamageType& value) const;
    void GetWeaponDamage(int32_t& value, bool critical);
    AttributeIndices GetWeaponAttribute() const;
    uint32_t GetWeaponRequirement() const;
    void GetArmor(DamageType damageType, int& value) const;
    void GetArmorPenetration(float& value) const;
    void GetAttributeValue(uint32_t index, uint32_t& value);

    AB::Entities::Item data_;
};

}
