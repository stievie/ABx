#pragma once

#include <memory>
#include "Item.h"
#include "Damage.h"
#include "Mechanic.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Account.h>
#include "ItemContainer.h"
#include "ItemsCache.h"

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
    EquipmentMap equipment_;
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

    EquipPos SetEquipment(uint32_t itemId);
    Item* GetEquipment(EquipPos pos) const;
    uint32_t RemoveEquipment(EquipPos pos);
    /// Swap inventory - equipment
    EquipPos EquipInventoryItem(ItemPos pos);

    bool SetInventoryItem(uint32_t itemId, Net::NetworkMessage* message);
    /// Remove and Destroy (i.e. delete from DB) the item
    bool DestroyInventoryItem(ItemPos pos)
    {
        return inventory_->DestroyItem(pos);
    }
    /// Removes the item, does not delete it, e.g. when dropped. Returns the item for further anything.
    /// Since it's a unique_ptr somebody should own it, if it's still needed.
    uint32_t RemoveInventoryItem(ItemPos pos)
    {
        return inventory_->RemoveItem(pos);
    }
    Item* GetInventoryItem(ItemPos pos)
    {
        return inventory_->GetItem(pos);
    }
    bool IsInventoryFull() const { return inventory_->IsFull(); }
    void SetInventorySize(size_t value)
    {
        inventory_->SetSize(value);
    }
    size_t GetInventoryCount() const
    {
        return inventory_->GetCount();
    }

    bool SetChestItem(uint32_t itemId, Net::NetworkMessage* message);
    /// Remove and Destroy (i.e. delete from DB) the item
    bool DestroyChestItem(ItemPos pos)
    {
        return chest_->DestroyItem(pos);
    }
    /// Removes the item, does not delete it, e.g. when dropped. Returns the item for further anything.
    /// Since it's a unique_ptr somebody should own it, if it's still needed.
    uint32_t RemoveChestItem(ItemPos pos)
    {
        return chest_->RemoveItem(pos);
    }
    Item* GetChestItem(ItemPos pos)
    {
        return chest_->GetItem(pos);
    }
    bool IsChestFull() const { return chest_->IsFull(); }
    void SetChestSize(size_t value)
    {
        chest_->SetSize(value);
    }
    size_t GetChestCount() const
    {
        return chest_->GetCount();
    }

    void SetUpgrade(Item& item, ItemUpgrade type, uint32_t upgradeId);
    void RemoveUpgrade(Item& item, ItemUpgrade type);

    /// Get lead hand weapon
    Item* GetWeapon() const;
    int GetArmor(DamageType damageType, DamagePos pos);
    float GetArmorPenetration();
    uint32_t GetAttributeValue(uint32_t index);
    void GetResources(int& maxHealth, int& maxEnergy);
    void GetSkillCost(Skill* skill, int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
    void GetSkillRecharge(Skill* skill, int32_t& recharge);
    template<typename Func>
    void VisitEquipement(const Func& func);
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

template<typename Func>
inline void InventoryComp::VisitEquipement(const Func& func)
{
    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : equipment_)
    {
        auto* item = cache->Get(i.second);
        if (item)
        {
            if (func(*item) != Iteration::Continue)
                break;
        }
    }
}

}
}
