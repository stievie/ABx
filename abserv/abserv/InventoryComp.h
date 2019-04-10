#pragma once

#include "Item.h"
#include "Damage.h"

namespace Game {

class Actor;

namespace Components {

static constexpr size_t DEFAULT_INV_SIZE = 12;

/// Equipment, like Armor, weapons, weapon mods etc.
class InventoryComp
{
private:
    Actor& owner_;
    std::map<EquipPos, std::unique_ptr<Item>> equipment_;
    std::vector<std::unique_ptr<Item>> inventory_;
public:
    InventoryComp() = delete;
    explicit InventoryComp(Actor& owner) :
        owner_(owner)
    {
        SetInventorySize(DEFAULT_INV_SIZE);
    }
    // non-copyable
    InventoryComp(const InventoryComp&) = delete;
    InventoryComp& operator=(const InventoryComp&) = delete;
    ~InventoryComp() = default;

    size_t GetInventorySize() const { return inventory_.size(); }
    void SetInventorySize(size_t value) { inventory_.resize(value); }
    void Update(uint32_t timeElapsed);

    void SetEquipment(std::unique_ptr<Item> item);
    Item* GetEquipment(EquipPos pos) const;
    void RemoveEquipment(EquipPos pos);

    bool SetInventory(std::unique_ptr<Item> item);
    void RemoveInventory(uint16_t pos);

    void SetUpgrade(Item* item, ItemUpgrade type, std::unique_ptr<Item> upgrade);
    void RemoveUpgrade(Item* item, ItemUpgrade type);

    /// Get lead hand weapon
    Item* GetWeapon() const;
    int GetArmor(DamageType damageType, DamagePos pos);
    float GetArmorPenetration();
    uint32_t GetAttributeValue(uint32_t index);

    template<typename Func>
    void VisitEquipement(Func&& func)
    {
        for (const auto& o : equipment_)
        {
            if (o.second)
                func(o.second.get());
        }
    }
    template<typename Func>
    void VisitInventory(Func&& func)
    {
        for (const auto& o : inventory_)
        {
            if (o)
                func(o.get());
        }
    }

};

}
}
