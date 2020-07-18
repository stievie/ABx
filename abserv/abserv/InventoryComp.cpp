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


#include "InventoryComp.h"
#include "ItemFactory.h"
#include "Player.h"
#include "Skill.h"
#include <sa/Transaction.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <abscommon/Logger.h>
#include <AB/Entities/MerchantItemList.h>
#include <sa/Transaction.h>

namespace Game {
namespace Components {

void InventoryComp::WriteItemUpdate(const Item* const item, Net::NetworkMessage* message)
{
    if (!item || !message)
        return;
    switch (item->concreteItem_.storagePlace)
    {
    case AB::Entities::StoragePlace::Inventory:
        message->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemUpdate);
        break;
    case AB::Entities::StoragePlace::Chest:
        message->AddByte(AB::GameProtocol::ServerPacketType::ChestItemUpdate);
        break;
    default:
        LOG_ERROR << "Unexpected storage place " << static_cast<int>(item->concreteItem_.storagePlace) << std::endl;
        return;
    }
    // TODO: Upgrades are missing.
    AB::Packets::Server::InventoryItemUpdate packet = {{
        item->data_.index,
        static_cast<uint16_t>(item->data_.type),
        item->concreteItem_.count,
        item->concreteItem_.value,
        item->concreteItem_.itemStats,
        static_cast<uint8_t>(item->concreteItem_.storagePlace),
        item->concreteItem_.storagePos,
        item->concreteItem_.flags
    }};
    AB::Packets::Add(packet, *message);
}

void InventoryComp::ExchangeItem(Item& item, uint32_t count,
    Player& removeFrom, Player& addTo,
    Net::NetworkMessage& removeMessage,
    Net::NetworkMessage& addMessage)
{
    auto* factory = GetSubsystem<ItemFactory>();
    InventoryComp& removeInv = *removeFrom.inventoryComp_;
    InventoryComp& addtoInv = *addTo.inventoryComp_;
    if (item.concreteItem_.count == count)
    {
        // Shortcut, just move the whole item including upgrades.
        uint32_t id = removeInv.RemoveInventoryItem(item.concreteItem_.storagePos);
        removeMessage.AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
        AB::Packets::Server::InventoryItemDelete packet = {
            item.concreteItem_.storagePos
        };
        AB::Packets::Add(packet, removeMessage);

        item.concreteItem_.accountUuid = addTo.GetAccountUuid();
        item.concreteItem_.playerUuid = addTo.data_.uuid;
        // Use next free slot
        item.concreteItem_.storagePos = 0;
        for (size_t i = 0; i < static_cast<size_t>(ItemUpgrade::__Count); ++i)
        {
            auto* upgrade = item.GetUpgrade(static_cast<ItemUpgrade>(i));
            if (!upgrade)
                continue;
            upgrade->concreteItem_.accountUuid = addTo.GetAccountUuid();
            upgrade->concreteItem_.playerUuid = addTo.data_.uuid;
        }
        addtoInv.SetInventoryItem(id, &addMessage);

        return;
    }

    // We must split the stack into 2 items.
    // This also means the item is stackable and therefore can not have upgrades.
    uint32_t itemId = factory->CreatePlayerItem(addTo, item.data_.uuid, AB::Entities::StoragePlace::Inventory, count);
    addtoInv.SetInventoryItem(itemId, &addMessage);

    item.concreteItem_.count -= count;
    InventoryComp::WriteItemUpdate(&item, &removeMessage);
}

InventoryComp::InventoryComp(Actor& owner) :
    owner_(owner),
    inventory_(ea::make_unique<ItemContainer>(MAX_INVENTORY_STACK_SIZE,
        AB::Entities::DEFAULT_INVENTORY_SIZE, MAX_INVENTOREY_MONEY,
        AB::Entities::StoragePlace::Inventory)),
    chest_(ea::make_unique<ItemContainer>(MAX_CHEST_STACK_SIZE,
        AB::Entities::DEFAULT_CHEST_SIZE, DEFAULT_CHEST_MONEY,
        AB::Entities::StoragePlace::Chest))
{ }

void InventoryComp::Update(uint32_t timeElapsed)
{
    VisitEquipement([&](Item& item)
    {
        item.Update(timeElapsed);
        return Iteration::Continue;
    });
}

EquipPos InventoryComp::SetEquipment(uint32_t itemId)
{
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (!item)
        return EquipPos::None;

    const EquipPos pos = item->GetEquipPos();
    {
        uint32_t old = RemoveEquipment(pos);
        if (old)
        {
            if (!owner_.AddToInventory(old))
            {
                return EquipPos::None;
            }
            auto* oldItem = cache->Get(old);
            if (oldItem)
                oldItem->OnUnequip(&owner_);
        }
    }
    equipment_[pos] = itemId;
    item->OnEquip(&owner_);
    return pos;
}

Item* InventoryComp::GetEquipment(EquipPos pos) const
{
    auto item = equipment_.find(pos);
    if (item == equipment_.end() || (*item).second == 0)
        return nullptr;

    auto* cache = GetSubsystem<ItemsCache>();
    return cache->Get((*item).second);
}

uint32_t InventoryComp::RemoveEquipment(EquipPos pos)
{
    if (equipment_[pos] == 0)
        return 0;
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(equipment_[pos]);
    if (!item)
        return 0;

    item->OnUnequip(&owner_);
    equipment_[pos] = 0;
    return item->id_;
}

EquipPos InventoryComp::EquipInventoryItem(ItemPos pos)
{
    Item* pItem = GetInventoryItem(pos);
    if (!pItem || pItem->GetEquipPos() == EquipPos::None)
        return EquipPos::None;
    uint32_t item = RemoveInventoryItem(pos);
    EquipPos ePos = SetEquipment(item);
    if (ePos != EquipPos::None)
        return ePos;

    // Rollback
    inventory_->InternalSetItem(item);
    return EquipPos::None;
}

bool InventoryComp::SetInventoryItem(uint32_t itemId, Net::NetworkMessage* message, uint16_t newPos)
{
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (!item)
        return false;

    item->concreteItem_.playerUuid = owner_.GetPlayerUuid();
    item->concreteItem_.accountUuid = owner_.GetAccountUuid();
    if (newPos != 0)
        item->concreteItem_.storagePos = newPos;
    const bool ret = inventory_->SetItem(itemId, [message](const Item* const item)
    {
        InventoryComp::WriteItemUpdate(item, message);
    });
    if (!ret)
        owner_.CallEvent<void(void)>(EVENT_ON_INVENTORYFULL);
    return ret;
}

bool InventoryComp::SellItem(ItemPos pos, uint32_t count, uint32_t pricePer, Net::NetworkMessage* message)
{
    ASSERT(Is<Player>(owner_));

    if (pos == 0)
        // Can not sell money
        return false;
    if (count == 0)
        return false;
    // Only items in inventory can be sold
    auto* item = inventory_->GetItem(pos);
    if (!item)
        return false;
    if (item->concreteItem_.count < count)
    {
        LOG_WARNING << "CHEAT: Player " << owner_.GetName() << " tries to sell more items " << count <<
            " than available " << item->concreteItem_.count << std::endl;
        return false;
    }

    if (!AB::Entities::IsItemTradeable(item->data_.itemFlags))
        return false;
    uint32_t amount = pricePer * count;
    if (amount == 0)
        return false;
    if (!CheckInventoryCapacity(amount, 0))
    {
        owner_.CallEvent<void(void)>(EVENT_ON_INVENTORYFULL);
        return false;
    }

    auto* factory = GetSubsystem<ItemFactory>();
    if (item->concreteItem_.count == count)
    {
        // Sell all
        if (!factory->MoveToMerchant(item, count))
            return false;
        RemoveInventoryItem(pos);

        if (message)
        {
            message->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
            AB::Packets::Server::InventoryItemDelete packet{
                pos
            };
            AB::Packets::Add(packet, *message);
        }
    }
    else
    {
        if (!factory->MoveToMerchant(item, count))
            return false;
        InventoryComp::WriteItemUpdate(item, message);
    }

    AddInventoryMoney(amount, message);
    return true;
}

bool InventoryComp::BuyItem(Item* item, uint32_t count, uint32_t pricePer, Net::NetworkMessage* message)
{
    ASSERT(Is<Player>(owner_));
    ASSERT(item);

    if (!AB::Entities::IsItemTradeable(item->data_.itemFlags))
        return false;
    if (!AB::Entities::IsItemResellable(item->data_.itemFlags))
        return false;

    auto* dc = GetSubsystem<IO::DataClient>();

    auto itemNotAvail = [&]()
    {
        // Meanwhile the item may be sold to another player
        if (!message)
            return;
        message->AddByte(AB::GameProtocol::ServerPacketType::PlayerError);
        AB::Packets::Server::PlayerError packet = {
            static_cast<uint8_t>(AB::GameProtocol::PlayerErrorValue::ItemNotAvailable)
        };
        AB::Packets::Add(packet, *message);
    };

    // Note: To avoid data races we lock the entity for writing for other clients
    // (i.e. game servers) while the transaction is in progress.
    IO::EntityLocker locker(*dc, item->concreteItem_);
    if (!locker.Lock())
    {
        // This can also happen when the item was sold already
        itemNotAvail();
        return false;
    }
    // Re-read the item, in case it was sold meanwhile to a different player.
    if (!dc->Read(item->concreteItem_))
    {
        itemNotAvail();
        return false;
    }
    if (item->concreteItem_.count == 0)
    {
        itemNotAvail();
        return false;
    }
    if (item->concreteItem_.storagePlace != AB::Entities::StoragePlace::Merchant)
    {
        itemNotAvail();
        return false;
    }

    // It may be possible that the player requests more items than the merchant owns.
    // But count must always be smaller or equal than available.
    if (count > item->concreteItem_.count)
        count = item->concreteItem_.count;

    uint32_t amount = pricePer * count;
    if (amount == 0)
        return false;

    if (!CheckInventoryCapacity(0, 1))
    {
        owner_.CallEvent<void(void)>(EVENT_ON_INVENTORYFULL);
        return false;
    }

    auto& player = To<Player>(owner_);

    if (!item->IsStackable() || item->concreteItem_.count == count)
    {
        sa::Transaction transaction(item->concreteItem_);
        // If the item is not stackable just move to the player
        item->concreteItem_.accountUuid = player.account_.uuid;
        item->concreteItem_.playerUuid = player.data_.uuid;
        item->concreteItem_.storagePlace = AB::Entities::StoragePlace::Inventory;
        if (!SetInventoryItem(item->id_, message))
            return false;
        dc->Update(item->concreteItem_);
        dc->Invalidate(item->concreteItem_);
        // Merchant got new items.
        AB::Entities::MerchantItemList ml;
        dc->Invalidate(ml);
    }
    else
    {
        sa::Transaction transaction(item->concreteItem_);
        auto* newItem = SplitStack(item, count, AB::Entities::StoragePlace::Inventory, 0);
        ASSERT(newItem);
        if (!SetInventoryItem(newItem->id_, message))
        {
            item->RemoveFromCache();
            return false;
        }
        transaction.Commit();
        dc->Update(item->concreteItem_);
        dc->Invalidate(item->concreteItem_);
    }

    RemoveInventoryMoney(amount, message);
    return true;
}

Item* InventoryComp::SplitStack(Item* item, uint32_t count, AB::Entities::StoragePlace newItemPlace, uint16_t newItemPos)
{
    ASSERT(Is<Player>(owner_));
    ASSERT(count > 0);
    // Count must be less than available, otherwise the whole stack should be moved
    if (item->concreteItem_.count <= count)
        return nullptr;

    auto* cache = GetSubsystem<ItemsCache>();
    auto* factory = GetSubsystem<ItemFactory>();
    uint32_t itemId = factory->CreatePlayerItem(To<Player>(owner_), item->data_.uuid, newItemPlace, count);
    item->concreteItem_.count -= count;
    auto* newItem = cache->Get(itemId);
    // Splitting a stack should never fail
    ASSERT(newItem);
    newItem->concreteItem_.storagePos = newItemPos;
    return newItem;
}

bool InventoryComp::SetChestItem(uint32_t itemId, Net::NetworkMessage* message, uint16_t newPos)
{
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (!item)
        return false;

    item->concreteItem_.playerUuid = owner_.GetPlayerUuid();
    item->concreteItem_.accountUuid = owner_.GetAccountUuid();
    if (newPos != 0)
        item->concreteItem_.storagePos = newPos;
    const bool ret = chest_->SetItem(itemId, [message](const Item* const item)
    {
        InventoryComp::WriteItemUpdate(item, message);
    });
    if (!ret)
        owner_.CallEvent<void(void)>(EVENT_ON_CHESTFULL);
    return ret;
}

uint32_t InventoryComp::AddChestMoney(uint32_t amount, Net::NetworkMessage* message)
{
    if (!Is<Player>(owner_))
        return 0;
    if (amount > static_cast<uint32_t>(chest_->GetMaxMoney()))
        return 0;

    auto* money = chest_->GetItem(0);
    if (money)
    {
        uint32_t maxadd = std::min(amount, static_cast<uint32_t>(chest_->GetMaxMoney()) - money->concreteItem_.count);
        money->concreteItem_.count += maxadd;
        InventoryComp::WriteItemUpdate(money, message);
        return maxadd;
    }

    auto* factory = GetSubsystem<ItemFactory>();
    uint32_t moneyId = factory->CreatePlayerMoneyItem(To<Player>(owner_), amount);
    if (!SetChestItem(moneyId, message))
        return 0;
    return amount;
}

uint32_t InventoryComp::RemoveChestMoney(uint32_t amount, Net::NetworkMessage* message)
{
    if (!Is<Player>(owner_))
        return 0;

    auto* money = chest_->GetItem(0);
    if (money)
    {
        uint32_t remove = std::min(amount, money->concreteItem_.count);
        money->concreteItem_.count -= remove;
        InventoryComp::WriteItemUpdate(money, message);
        return remove;
    }
    return 0;
}

uint32_t InventoryComp::AddInventoryMoney(uint32_t amount, Net::NetworkMessage* message)
{
    if (!Is<Player>(owner_))
        return 0;
    if (amount > static_cast<uint32_t>(inventory_->GetMaxMoney()))
        return 0;

    auto* money = inventory_->GetItem(0);
    if (money)
    {
        uint32_t maxadd = std::min(amount, static_cast<uint32_t>(inventory_->GetMaxMoney()) - money->concreteItem_.count);
        money->concreteItem_.count += maxadd;
        InventoryComp::WriteItemUpdate(money, message);
        return maxadd;
    }

    auto* factory = GetSubsystem<ItemFactory>();
    uint32_t moneyId = factory->CreatePlayerMoneyItem(To<Player>(owner_), amount);
    if (!SetInventoryItem(moneyId, message))
        return 0;
    return amount;
}

uint32_t InventoryComp::RemoveInventoryMoney(uint32_t amount, Net::NetworkMessage* message)
{
    if (!Is<Player>(owner_))
        return 0;

    auto* money = inventory_->GetItem(0);
    if (money)
    {
        uint32_t remove = std::min(amount, money->concreteItem_.count);
        money->concreteItem_.count -= remove;
        InventoryComp::WriteItemUpdate(money, message);
        return remove;
    }
    return 0;
}

uint32_t InventoryComp::DepositMoney(uint32_t amount, Net::NetworkMessage* message)
{
    if (!Is<Player>(owner_))
        return 0;
    if (amount > static_cast<uint32_t>(chest_->GetMaxMoney()))
        return 0;
    auto* invmoney = inventory_->GetItem(0);
    if (!invmoney)
        return 0;

    uint32_t maxadd = std::min(static_cast<uint32_t>(chest_->GetMaxMoney()), amount);
    if (maxadd > inventory_->GetMoney())
        maxadd = inventory_->GetMoney();

    auto* chestmoney = chest_->GetItem(0);
    if (!chestmoney)
    {
        auto* factory = GetSubsystem<ItemFactory>();
        uint32_t moneyId = factory->CreatePlayerMoneyItem(To<Player>(owner_), maxadd);
        if (!SetChestItem(moneyId, message))
            return 0;
    }
    else
    {
        maxadd = std::min(amount, static_cast<uint32_t>(chest_->GetMaxMoney()) - chestmoney->concreteItem_.count);
        chestmoney->concreteItem_.count += maxadd;
        InventoryComp::WriteItemUpdate(chestmoney, message);
    }
    invmoney->concreteItem_.count -= maxadd;
    InventoryComp::WriteItemUpdate(invmoney, message);

    return maxadd;
}

uint32_t InventoryComp::WithdrawMoney(uint32_t amount, Net::NetworkMessage* message)
{
    if (!Is<Player>(owner_))
        return 0;
    if (amount > static_cast<uint32_t>(inventory_->GetMaxMoney()))
        return 0;
    auto* chestmoney = chest_->GetItem(0);
    if (!chestmoney)
        return 0;

    uint32_t maxadd = std::min(static_cast<uint32_t>(inventory_->GetMaxMoney()), amount);
    if (maxadd > chest_->GetMoney())
        maxadd = chest_->GetMoney();

    auto* invmoney = inventory_->GetItem(0);
    if (!invmoney)
    {
        auto* factory = GetSubsystem<ItemFactory>();
        uint32_t moneyId = factory->CreatePlayerMoneyItem(To<Player>(owner_), maxadd);
        if (!SetInventoryItem(moneyId, message))
            return 0;
    }
    else
    {
        maxadd = std::min(amount, static_cast<uint32_t>(inventory_->GetMaxMoney()) - invmoney->concreteItem_.count);
        invmoney->concreteItem_.count += maxadd;
        InventoryComp::WriteItemUpdate(invmoney, message);
    }
    chestmoney->concreteItem_.count -= maxadd;
    InventoryComp::WriteItemUpdate(chestmoney, message);

    return maxadd;
}

uint32_t InventoryComp::GetChestMoney() const
{
    return chest_->GetMoney();
}

uint32_t InventoryComp::GetInventoryMoney() const
{
    return inventory_->GetMoney();
}

bool InventoryComp::HaveInventoryItem(uint32_t itemIndex, uint32_t count)
{
    uint32_t availCount = 0;
    VisitInventory([&](const Item& current)
    {
        if (current.data_.index == itemIndex)
            availCount += current.concreteItem_.count;
        return Iteration::Continue;
    });
    return availCount >= count;
}

bool InventoryComp::TakeInventoryItem(uint32_t itemIndex, uint32_t count, Net::NetworkMessage* message)
{
    // Make sure the inventory contains enough before calling this function
    uint32_t remaining = count;
    ea::set<uint32_t> deleted;
    VisitInventory([&](Item& current)
    {
        if (current.data_.index == itemIndex)
        {
            if (current.concreteItem_.count < remaining)
            {
                remaining -= current.concreteItem_.count;
                current.concreteItem_.count = 0;
            }
            else
            {
                current.concreteItem_.count -= remaining;
                remaining = 0;
            }
            if (current.concreteItem_.count == 0)
            {
                if (message)
                {
                    message->AddByte(AB::GameProtocol::ServerPacketType::InventoryItemDelete);
                    AB::Packets::Server::InventoryItemDelete packet = {
                        current.concreteItem_.storagePos
                    };
                }
                deleted.emplace(current.id_);
            }
            else
            {
                WriteItemUpdate(&current, message);
            }
        }
        if (remaining > 0)
            return Iteration::Continue;
        return Iteration::Break;
    });

    if (deleted.size() != 0)
    {
        auto* cache = GetSubsystem<ItemsCache>();
        auto* factory = GetSubsystem<ItemFactory>();
        for (auto id : deleted)
        {
            auto* pItem = cache->Get(id);
            ASSERT(pItem);
            factory->DeleteItem(pItem);
        }
    }
    return remaining == 0;
}

void InventoryComp::SetUpgrade(Item& item, ItemUpgrade type, uint32_t upgradeId)
{
    const bool isEquipped = item.concreteItem_.storagePlace == AB::Entities::StoragePlace::Equipped;
    if (isEquipped)
    {
        Item* old = item.GetUpgrade(type);
        if (old)
            old->OnUnequip(&owner_);
    }
    Item* n = item.SetUpgrade(type, upgradeId);
    if (n && isEquipped)
        n->OnEquip(&owner_);
}

void InventoryComp::RemoveUpgrade(Item& item, ItemUpgrade type)
{
    const bool isEquipped = item.concreteItem_.storagePlace == AB::Entities::StoragePlace::Equipped;
    Item* old = item.GetUpgrade(type);
    if (old)
    {
        if (isEquipped)
            old->OnUnequip(&owner_);
        item.RemoveUpgrade(type);
    }
}

Item* InventoryComp::GetWeapon() const
{
    Item* result = GetEquipment(EquipPos::WeaponLeadHand);
    if (!result)
        result = GetEquipment(EquipPos::WeaponTwoHanded);
    return result;
}

int InventoryComp::GetArmor(DamageType damageType, DamagePos pos)
{
    Item* item = nullptr;
    auto* cache = GetSubsystem<ItemsCache>();
    switch (pos)
    {
    case DamagePos::Head:
        item = cache->Get(equipment_[EquipPos::ArmorHead]);
        break;
    case DamagePos::Chest:
        item = cache->Get(equipment_[EquipPos::ArmorChest]);
        break;
    case DamagePos::Hands:
        item = cache->Get(equipment_[EquipPos::ArmorHands]);
        break;
    case DamagePos::Legs:
        item = cache->Get(equipment_[EquipPos::ArmorLegs]);
        break;
    case DamagePos::Feet:
        item = cache->Get(equipment_[EquipPos::ArmorFeet]);
        break;
    case DamagePos::NoPos:
        return 0;
    default:
        break;
    }
    if (!item)
        return 0;
    int armor = 0;
    item->GetArmor(damageType, armor);
    return armor;
}

float InventoryComp::GetArmorPenetration()
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return 0.0f;
    float value = 0.0f;
    weapon->GetArmorPenetration(value);
    return value;
}

uint32_t InventoryComp::GetAttributeRank(Attribute index)
{
    uint32_t result = 0;
    VisitEquipement([&](Item& item)
    {
        item.GetAttributeRank(index, result);
        return Iteration::Continue;
    });
    return result;
}

void InventoryComp::GetResources(int& maxHealth, int& maxEnergy)
{
    VisitEquipement([&](Item& item)
    {
        item.GetResources(maxHealth, maxEnergy);
        return Iteration::Continue;
    });
}

void InventoryComp::GetSkillRecharge(Skill* skill, uint32_t &recharge)
{
    VisitEquipement([&](Item& item)
    {
        item.GetSkillRecharge(skill, recharge);
        return Iteration::Continue;
    });
}

void InventoryComp::GetSkillCost(Skill* skill, int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp)
{
    VisitEquipement([&](Item& item)
    {
        item.GetSkillCost(skill, activation, energy, adrenaline, overcast, hp);
        return Iteration::Continue;
    });
}

}
}
