#pragma once

#include <memory>
#include "Item.h"
#include "Damage.h"
#include "Mechanic.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Account.h>
#include "ItemContainer.h"

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;
class Skill;

namespace Components {

/// Equipment, like Armor, weapons, weapon mods etc.
class InventoryComp
{
private:
    Actor& owner_;
    std::map<EquipPos, std::unique_ptr<Item>> equipment_;
    /// Character inventory
    std::unique_ptr<ItemContainer> inventory_;
    /// Account chest
    std::unique_ptr<ItemContainer> chest_;
    static void WriteItemUpdate(const Item* const item, Net::NetworkMessage* message, bool isChest);
public:
    InventoryComp() = delete;
    explicit InventoryComp(Actor& owner) :
        owner_(owner),
        inventory_(std::make_unique<ItemContainer>(MAX_INVENTORY_STACK_SIZE, AB::Entities::DEFAULT_INVENTORY_SIZE, AB::Entities::StoragePlaceInventory)),
        chest_(std::make_unique<ItemContainer>(MAX_CHEST_STACK_SIZE, AB::Entities::DEFAULT_CHEST_SIZE, AB::Entities::StoragePlaceChest))
    { }
    // non-copyable
    InventoryComp(const InventoryComp&) = delete;
    InventoryComp& operator=(const InventoryComp&) = delete;
    ~InventoryComp() = default;

    void Update(uint32_t timeElapsed);

    EquipPos SetEquipment(std::unique_ptr<Item>& item);
    Item* GetEquipment(EquipPos pos) const;
    std::unique_ptr<Item> RemoveEquipment(EquipPos pos);
    /// Swap inventory - equipment
    EquipPos EquipInventoryItem(uint16_t pos);

    bool SetInventoryItem(std::unique_ptr<Item>& item, Net::NetworkMessage* message);
    /// Remove and Destroy (i.e. delete from DB) the item
    bool DestroyInventoryItem(uint16_t pos)
    {
        return inventory_->DestroyItem(pos);
    }
    /// Removes the item, does not delete it, e.g. when dropped. Returns the item for further anything.
    /// Since it's a unique_ptr somebody should own it, if it's still needed.
    std::unique_ptr<Item> RemoveInventoryItem(uint16_t pos)
    {
        return inventory_->RemoveItem(pos);
    }
    Item* GetInventoryItem(uint16_t pos)
    {
        return inventory_->GetItem(pos);
    }
    bool IsInventoryFull() const { return inventory_->IsFull(); }
    void SetInventorySize(uint16_t value)
    {
        inventory_->SetSize(value);
    }
    size_t GetInventoryCount() const
    {
        return inventory_->GetCount();
    }

    bool SetChestItem(std::unique_ptr<Item>& item, Net::NetworkMessage* message);
    /// Remove and Destroy (i.e. delete from DB) the item
    bool DestroyChestItem(uint16_t pos)
    {
        return chest_->DestroyItem(pos);
    }
    /// Removes the item, does not delete it, e.g. when dropped. Returns the item for further anything.
    /// Since it's a unique_ptr somebody should own it, if it's still needed.
    std::unique_ptr<Item> RemoveChestItem(uint16_t pos)
    {
        return chest_->RemoveItem(pos);
    }
    Item* GetChestItem(uint16_t pos)
    {
        return chest_->GetItem(pos);
    }
    bool IsChestFull() const { return chest_->IsFull(); }
    void SetChestSize(uint16_t value)
    {
        chest_->SetSize(value);
    }
    size_t GetChestCount() const
    {
        return chest_->GetCount();
    }

    void SetUpgrade(Item* item, ItemUpgrade type, std::unique_ptr<Item> upgrade);
    void RemoveUpgrade(Item* item, ItemUpgrade type);

    /// Get lead hand weapon
    Item* GetWeapon() const;
    int GetArmor(DamageType damageType, DamagePos pos);
    float GetArmorPenetration();
    uint32_t GetAttributeValue(uint32_t index);
    void GetResources(int& maxHealth, int& maxEnergy);
    void GetSkillCost(Skill* skill, int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
    template<typename Func>
    void VisitEquipement(const Func& func)
    {
        for (const auto& o : equipment_)
        {
            if (o.second)
                func(o.second.get());
        }
    }
    template<typename Func>
    void VisitInventory(const Func& func)
    {
        inventory_->VisitItems(func);
    }
    template<typename Func>
    void VisitChest(const Func& func)
    {
        chest_->VisitItems(func);
    }

};

}
}
