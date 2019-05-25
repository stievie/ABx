#pragma once

#include "Item.h"
#include "Damage.h"
#include "Mechanic.h"
#include <AB/Entities/Character.h>
#include "ItemContainer.h"

namespace Game {

class Actor;

namespace Components {

/// Equipment, like Armor, weapons, weapon mods etc.
class InventoryComp
{
private:
    Actor& owner_;
    std::map<EquipPos, std::unique_ptr<Item>> equipment_;
    std::unique_ptr<ItemContainer> inventory_;
    static void WriteItemUpdate(const Item* const item, Net::NetworkMessage* message);
public:
    InventoryComp() = delete;
    explicit InventoryComp(Actor& owner) :
        owner_(owner),
        inventory_(std::make_unique<ItemContainer>(MAX_INVENTORY_STACK_SIZE, AB::Entities::DEFAULT_INVENTORY_SIZE))
    { }
    // non-copyable
    InventoryComp(const InventoryComp&) = delete;
    InventoryComp& operator=(const InventoryComp&) = delete;
    ~InventoryComp() = default;

    void Update(uint32_t timeElapsed);

    void SetEquipment(std::unique_ptr<Item> item);
    Item* GetEquipment(EquipPos pos) const;
    std::unique_ptr<Item> RemoveEquipment(EquipPos pos);

    bool SetInventory(std::unique_ptr<Item>& item, Net::NetworkMessage* message);
    /// Remove and Destroy (i.e. delete from DB) the item
    bool DestroyItem(uint16_t pos)
    {
        return inventory_->DestroyItem(pos);
    }
    /// Removes the item, does not delete it, e.g. when dropped. Returns the item for further anything.
    /// Since it's a unique_ptr somebody should own it, if it's still needed.
    std::unique_ptr<Item> RemoveItem(uint16_t pos)
    {
        return inventory_->RemoveItem(pos);
    }
    Item* GetItem(uint16_t pos)
    {
        return inventory_->GetItem(pos);
    }
    bool IsFull() const { return inventory_->IsFull(); }
    void SetSize(uint16_t value)
    {
        inventory_->SetSize(value);
    }
    size_t GetCount() const
    {
        return inventory_->GetCount();
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
        inventory_->VisitItems(func);
    }

};

}
}
