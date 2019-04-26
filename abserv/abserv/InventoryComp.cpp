#include "stdafx.h"
#include "InventoryComp.h"
#include "IOItem.h"
#include "ItemFactory.h"
#include "Actor.h"
#include "DataClient.h"

namespace Game {
namespace Components {

void InventoryComp::Update(uint32_t timeElapsed)
{
    for (const auto& item : equipment_)
    {
        if (item.second)
            item.second->Update(timeElapsed);
    }
}

void InventoryComp::SetEquipment(std::unique_ptr<Item> item)
{
    if (item)
    {
        EquipPos pos = static_cast<EquipPos>(item->concreteItem_.storagePos);
        RemoveEquipment(pos);
        equipment_[pos] = std::move(item);
        equipment_[pos]->OnEquip(&owner_);
    }
}

Item* InventoryComp::GetEquipment(EquipPos pos) const
{
    auto item = equipment_.find(pos);
    if (item == equipment_.end() || !(*item).second)
        return nullptr;

    return (*item).second.get();
}

void InventoryComp::RemoveEquipment(EquipPos pos)
{
    if (!equipment_[pos])
        return;
    equipment_[pos]->OnUnequip(&owner_);
    equipment_.erase(pos);
}

Item* InventoryComp::AddInventory(std::unique_ptr<Item>& item)
{

    // pos = 1-based
    size_t pos = item->concreteItem_.storagePos;
    if (pos == 0)
    {
        if (item->data_.stackAble)
        {
            Item* invItem = FindItem(item->data_.uuid);
            if (invItem)
            {
                // If this item is stackable merge them
                if (invItem->concreteItem_.count + item->concreteItem_.count < MAX_INVENTORY_STACK_SIZE)
                {
                    invItem->concreteItem_.count += item->concreteItem_.count;
                    // Merged -> delete this
                    auto factory = GetSubsystem<ItemFactory>();
                    factory->DeleteConcrete(item->concreteItem_.uuid);
                    return invItem;
                }
            }
        }

        if (GetCount() < inventorySize_)
        {
            int16_t p = InsertItem(item);
            if (p != 0)
            {
                return GetItem(p);
            }
            return nullptr;
        }
        // Inventory full
        return nullptr;
    }
    else
    {
        if (inventory_.size() > pos)
        {
            if (!inventory_[pos])
            {
                inventory_[pos] = std::move(item);
                return inventory_[pos].get();
            }
        }
        if (inventory_.size() < inventorySize_)
        {
            if (inventory_.size() <= pos)
                inventory_.resize(pos + 1);
            inventory_[pos] = std::move(item);
            return inventory_[pos].get();
        }
        return nullptr;
    }
}

Item* InventoryComp::FindItem(const std::string& uuid)
{
    for (const auto& i : inventory_)
    {
        if (i && i->data_.uuid.compare(uuid) == 0)
            return i.get();
    }
    return nullptr;
}

uint16_t InventoryComp::InsertItem(std::unique_ptr<Item>& item)
{
    for (size_t i = 1; i < inventory_.size(); ++i)
    {
        if (!inventory_[i])
        {
            inventory_[i] = std::move(item);
            inventory_[i]->concreteItem_.storagePlace = AB::Entities::StoragePlaceInventory;
            inventory_[i]->concreteItem_.storagePos = static_cast<uint16_t>(i);
            return static_cast<uint16_t>(i);
        }
    }
    // No free slot between
    if (inventory_.size() < inventorySize_)
    {
        inventory_.push_back(std::move(item));
        uint16_t pos = static_cast<uint16_t>(inventory_.size()) - 1;
        inventory_[pos]->concreteItem_.storagePlace = AB::Entities::StoragePlaceInventory;
        inventory_[pos]->concreteItem_.storagePos = static_cast<uint16_t>(pos);
        return pos;
    }
    return 0;
}

Item* InventoryComp::SetInventory(std::unique_ptr<Item>& item)
{
    if (item)
    {
        if (item->data_.type == AB::Entities::ItemTypeMoney)
        {
            if (!inventory_[0])
            {
                item->concreteItem_.storagePlace = AB::Entities::StoragePlaceInventory;
                inventory_[0] = std::move(item);
            }
            else
            {
                if (inventory_[0]->concreteItem_.count + item->concreteItem_.count > MAX_INVENTOREY_MONEY)
                    return false;
                inventory_[0]->concreteItem_.count += item->concreteItem_.count;
                // Merged -> delete this
                auto factory = GetSubsystem<ItemFactory>();
                factory->DeleteConcrete(item->concreteItem_.uuid);
            }
            return inventory_[0].get();
        }

        Item* res = AddInventory(item);
        if (!res)
            owner_.OnInventoryFull();
        return res;
    }
    return nullptr;
}

bool InventoryComp::DestroyItem(uint16_t pos)
{
    if (inventory_.size() > pos && inventory_[pos])
    {
        auto factory = GetSubsystem<ItemFactory>();
        factory->DeleteItem(inventory_[pos].get());
        inventory_[pos].reset();
        return true;
    }
    return false;
}

Item* InventoryComp::GetItem(uint16_t pos)
{
    if (inventory_.size() >= pos && inventory_[pos])
    {
        return inventory_[pos].get();
    }
    return nullptr;
}

void InventoryComp::SetUpgrade(Item* item, ItemUpgrade type, std::unique_ptr<Item> upgrade)
{
    const bool isEquipped = item->concreteItem_.storagePlace == AB::Entities::StoragePlaceEquipped;
    if (isEquipped)
    {
        Item* old = item->GetUpgrade(type);
        if (old)
            old->OnUnequip(&owner_);
    }
    Item* n = item->SetUpgrade(type, std::move(upgrade));
    if (n && isEquipped)
        n->OnEquip(&owner_);
}

void InventoryComp::RemoveUpgrade(Item* item, ItemUpgrade type)
{
    const bool isEquipped = item->concreteItem_.storagePlace == AB::Entities::StoragePlaceEquipped;
    Item* old = item->GetUpgrade(type);
    if (old)
    {
        if (isEquipped)
            old->OnUnequip(&owner_);
        item->RemoveUpgrade(type);
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
    switch (pos)
    {
    case DamagePos::Head:
        item = equipment_[EquipPos::ArmorHead].get();
        break;
    case DamagePos::Chest:
        item = equipment_[EquipPos::ArmorChest].get();
        break;
    case DamagePos::Hands:
        item = equipment_[EquipPos::ArmorHands].get();
        break;
    case DamagePos::Legs:
        item = equipment_[EquipPos::ArmorLegs].get();
        break;
    case DamagePos::Feet:
        item = equipment_[EquipPos::ArmorFeet].get();
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

uint32_t InventoryComp::GetAttributeValue(uint32_t index)
{
    uint32_t result = 0;
    for (const auto& item : equipment_)
    {
        if (item.second)
            item.second->GetAttributeValue(index, result);
    }
    return result;
}

void InventoryComp::GetResources(int& maxHealth, int& maxEnergy)
{
    for (const auto& item : equipment_)
    {
        if (item.second)
            item.second->GetResources(maxHealth, maxEnergy);
    }
}

}
}
