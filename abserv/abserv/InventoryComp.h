#pragma once

#include "Item.h"
#include "Damage.h"
#include "Mechanic.h"

namespace Game {

class Actor;

namespace Components {

/// Equipment, like Armor, weapons, weapon mods etc.
class InventoryComp
{
private:
    Actor& owner_;
    std::map<EquipPos, std::unique_ptr<Item>> equipment_;
    std::vector<std::unique_ptr<Item>> inventory_;
    std::unique_ptr<Item> moneyItem_;
    size_t inventorySize_;
    bool AddInventory(std::unique_ptr<Item>& item);
    Item* FindItem(const std::string& uuid);
    /// Insert in first free slot
    uint16_t InsertItem(std::unique_ptr<Item>& item);
public:
    InventoryComp() = delete;
    explicit InventoryComp(Actor& owner) :
        owner_(owner),
        inventorySize_(DEFAULT_INVENTORY_SIZE)
    { }
    // non-copyable
    InventoryComp(const InventoryComp&) = delete;
    InventoryComp& operator=(const InventoryComp&) = delete;
    ~InventoryComp() = default;

    void Update(uint32_t timeElapsed);

    void SetEquipment(std::unique_ptr<Item> item);
    Item* GetEquipment(EquipPos pos) const;
    void RemoveEquipment(EquipPos pos);

    bool SetInventory(std::unique_ptr<Item>& item);
    void RemoveInventory(uint16_t pos);
    bool DestroyItem(uint16_t pos);
    bool IsFull() const { return inventory_.size() >= inventorySize_; }
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
        if (moneyItem_)
            func(moneyItem_.get());

        for (const auto& o : inventory_)
        {
            if (o)
                func(o.get());
        }
    }

};

}
}
