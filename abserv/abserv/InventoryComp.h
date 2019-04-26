#pragma once

#include "Item.h"
#include "Damage.h"
#include "Mechanic.h"
#include <AB/Entities/Character.h>

namespace Game {

class Actor;

namespace Components {

/// Equipment, like Armor, weapons, weapon mods etc.
class InventoryComp
{
private:
    Actor& owner_;
    std::map<EquipPos, std::unique_ptr<Item>> equipment_;
    /// All inventory. Index 0 is the money
    std::vector<std::unique_ptr<Item>> inventory_;
    size_t inventorySize_;
    Item* AddInventory(std::unique_ptr<Item>& item);
    Item* FindItem(const std::string& uuid);
    /// Insert in first free slot
    uint16_t InsertItem(std::unique_ptr<Item>& item);
public:
    InventoryComp() = delete;
    explicit InventoryComp(Actor& owner) :
        owner_(owner),
        inventorySize_(AB::Entities::DEFAULT_INVENTORY_SIZE)
    { 
        // Money
        inventory_.resize(1);
    }
    // non-copyable
    InventoryComp(const InventoryComp&) = delete;
    InventoryComp& operator=(const InventoryComp&) = delete;
    ~InventoryComp() = default;

    void Update(uint32_t timeElapsed);

    void SetEquipment(std::unique_ptr<Item> item);
    Item* GetEquipment(EquipPos pos) const;
    void RemoveEquipment(EquipPos pos);

    Item* SetInventory(std::unique_ptr<Item>& item);
    bool DestroyItem(uint16_t pos);
    Item* GetItem(uint16_t pos);
    bool IsFull() const { return GetCount() >= inventorySize_; }
    void SetSize(uint16_t value)
    {
        // Can not resize inventory when it has items loaded
        assert(inventory_.size() == 0);
        inventorySize_ = value;
    }
    size_t GetCount() const
    {
        size_t count = 0;
        for (const auto& i : inventory_)
        {
            if (i)
                ++count;
        }
        return count;
    }

    void SetUpgrade(Item* item, ItemUpgrade type, std::unique_ptr<Item> upgrade);
    void RemoveUpgrade(Item* item, ItemUpgrade type);

    /// Get lead hand weapon
    Item* GetWeapon() const;
    int GetArmor(DamageType damageType, DamagePos pos);
    float GetArmorPenetration();
    uint32_t GetAttributeValue(uint32_t index);
    void GetResources(int& maxHealth, int& maxEnergy);

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
