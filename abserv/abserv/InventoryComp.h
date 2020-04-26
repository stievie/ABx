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
#include "Item.h"
#include <abshared/Damage.h>
#include <abshared/Mechanic.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Account.h>
#include "ItemContainer.h"
#include "ItemsCache.h"
#include <sa/Noncopyable.h>

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
    NON_COPYABLE(InventoryComp)
    NON_MOVEABLE(InventoryComp)
private:
    Actor& owner_;
    EquipmentMap equipment_;
    /// Character inventory
    std::unique_ptr<ItemContainer> inventory_;
    /// Account chest
    std::unique_ptr<ItemContainer> chest_;
public:
    static void WriteItemUpdate(const Item* const item, Net::NetworkMessage* message);
    // Move an Item from one player to another
    static void ExchangeItem(Item& item, uint32_t count,
        Player& removeFrom, Player& addTo,
        Net::NetworkMessage& removeMessage,
        Net::NetworkMessage& addMessage);

    InventoryComp() = delete;
    explicit InventoryComp(Actor& owner);
    ~InventoryComp() = default;

    void Update(uint32_t timeElapsed);

    EquipPos SetEquipment(uint32_t itemId);
    Item* GetEquipment(EquipPos pos) const;
    uint32_t RemoveEquipment(EquipPos pos);
    /// Swap inventory - equipment
    EquipPos EquipInventoryItem(ItemPos pos);

    bool SetInventoryItem(uint32_t itemId, Net::NetworkMessage* message,
        uint16_t newPos = 0);
    /// Remove and Destroy (i.e. delete from DB) the item
    bool DestroyInventoryItem(ItemPos pos)
    {
        return inventory_->DestroyItem(pos);
    }
    /// Removes the item, does not delete it, e.g. when dropped. Returns the item for further anything.
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
    size_t GetInventorySize() const { return inventory_->GetSize(); }
    size_t GetInventoryCount() const
    {
        return inventory_->GetCount();
    }
    size_t GetFreeInventorySpace() const { return inventory_->GetFreeSpace(); }
    size_t GetMaxInventoryMoney() const { return inventory_->GetMaxMoney(); }
    bool CheckInventoryCapacity(uint32_t money, size_t itemCount) const { return inventory_->CheckCapacity(money, itemCount); }
    bool SetChestItem(uint32_t itemId, Net::NetworkMessage* message,
        uint16_t newPos = 0);
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
    size_t GetChestSize() const { return chest_->GetSize(); }
    size_t GetChestCount() const
    {
        return chest_->GetCount();
    }
    size_t GetFreeChestSpace() const { return chest_->GetFreeSpace(); }
    size_t GetMaxChestMoney() const { return chest_->GetMaxMoney(); }
    bool CheckChestCapacity(uint32_t money, size_t itemCount) const { return chest_->CheckCapacity(money, itemCount); }
    uint32_t AddChestMoney(uint32_t amount, Net::NetworkMessage* message);
    // Return how much money was removed
    uint32_t RemoveChestMoney(uint32_t amount, Net::NetworkMessage* message);
    uint32_t AddInventoryMoney(uint32_t amount, Net::NetworkMessage* message);
    uint32_t RemoveInventoryMoney(uint32_t amount, Net::NetworkMessage* message);
    uint32_t DepositMoney(uint32_t amount, Net::NetworkMessage* message);
    uint32_t WithdrawMoney(uint32_t amount, Net::NetworkMessage* message);
    uint32_t GetChestMoney() const;
    uint32_t GetInventoryMoney() const;

    void SetUpgrade(Item& item, ItemUpgrade type, uint32_t upgradeId);
    void RemoveUpgrade(Item& item, ItemUpgrade type);

    /// Get lead hand weapon
    Item* GetWeapon() const;
    int GetArmor(DamageType damageType, DamagePos pos);
    float GetArmorPenetration();
    uint32_t GetAttributeRank(Attribute index);
    void GetResources(int& maxHealth, int& maxEnergy);
    void GetSkillCost(Skill* skill, int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp);
    void GetSkillRecharge(Skill* skill, uint32_t& recharge);
    template<typename Func>
    void VisitEquipement(Func&& func);
    template<typename Func>
    void VisitInventory(Func&& func)
    {
        inventory_->VisitItems(std::forward<Func>(func));
    }
    template<typename Func>
    void VisitChest(Func&& func)
    {
        chest_->VisitItems(std::forward<Func>(func));
    }
};

template<typename Func>
inline void InventoryComp::VisitEquipement(Func&& func)
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
