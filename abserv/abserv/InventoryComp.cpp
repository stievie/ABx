#include "stdafx.h"
#include "InventoryComp.h"
#include "IOItem.h"
#include "ItemFactory.h"
#include "Actor.h"
#include "DataClient.h"
#include "MathUtils.h"

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

std::unique_ptr<Item> InventoryComp::RemoveEquipment(EquipPos pos)
{
    if (!equipment_[pos])
        return std::unique_ptr<Item>();
    std::unique_ptr<Item> item = std::move(equipment_[pos]);
    item->OnUnequip(&owner_);
    return item;
}

void InventoryComp::WriteItemUpdate(const Item* const item, Net::NetworkMessage* message, bool isChest)
{
    if (!message)
        return;
    message->AddByte((!isChest) ? AB::GameProtocol::InventoryItemUpdate : AB::GameProtocol::ChestItemUpdate);
    message->Add<uint16_t>(item->data_.type);
    message->Add<uint32_t>(item->data_.index);
    message->Add<uint8_t>(static_cast<uint8_t>(item->concreteItem_.storagePlace));
    message->Add<uint16_t>(item->concreteItem_.storagePos);
    message->Add<uint32_t>(item->concreteItem_.count);
    message->Add<uint16_t>(item->concreteItem_.value);
}

bool InventoryComp::SetInventoryItem(std::unique_ptr<Item>& item, Net::NetworkMessage* message)
{
    item->concreteItem_.playerUuid = owner_.GetPlayerUuid();
    item->concreteItem_.accountUuid = owner_.GetAccountUuid();
    const bool ret = inventory_->SetItem(item, [message](const Item* const item)
    {
        InventoryComp::WriteItemUpdate(item, message, false);
    });
    if (!ret)
        owner_.OnInventoryFull();
    return ret;
}

bool InventoryComp::SetChestItem(std::unique_ptr<Item>& item, Net::NetworkMessage* message)
{
    item->concreteItem_.playerUuid = owner_.GetPlayerUuid();
    item->concreteItem_.accountUuid = owner_.GetAccountUuid();
    const bool ret = inventory_->SetItem(item, [message](const Item* const item)
    {
        InventoryComp::WriteItemUpdate(item, message, true);
    });
    if (!ret)
        owner_.OnInventoryFull();
    return ret;
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
