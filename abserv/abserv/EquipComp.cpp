#include "stdafx.h"
#include "EquipComp.h"
#include "IOItem.h"

namespace Game {
namespace Components {

void EquipComp::Update(uint32_t timeElapsed)
{
    for (const auto& item : items_)
    {
        item.second->Update(timeElapsed);
    }
}

void EquipComp::SetItem(std::unique_ptr<Item> item)
{
    if (item)
    {
        EquipPos pos = static_cast<EquipPos>(item->concreteItem_.storagePos);
        RemoveItem(pos);
        items_[pos] = std::move(item);
        items_[pos]->OnEquip(&owner_);
    }
}

Item* EquipComp::GetItem(EquipPos pos) const
{
    auto item = items_.find(pos);
    if (item == items_.end() || !(*item).second)
        return nullptr;

    return (*item).second.get();
}

std::vector<Item*> EquipComp::GetItems() const
{
    std::vector<Item*> result;
    for (const auto& i : items_)
        result.push_back(i.second.get());
    return result;
}

void EquipComp::RemoveItem(EquipPos pos)
{
    if (!items_[pos])
        return;
    items_[pos]->OnUnequip(&owner_);
    items_.erase(pos);
}

void EquipComp::SetUpgrade(EquipPos pos, ItemUpgrade type, uint32_t index)
{
    if (!items_[pos])
        return;
    Item* old = items_[pos]->GetUpgrade(type);
    if (old)
        old->OnUnequip(&owner_);
    Item* n = items_[pos]->SetUpgrade(type, index);
    if (n)
        n->OnEquip(&owner_);
}

void EquipComp::RemoveUpgrade(EquipPos pos, ItemUpgrade type)
{
    Item* old = items_[pos]->GetUpgrade(type);
    if (old)
    {
        old->OnUnequip(&owner_);
        items_[pos]->RemoveUpgrade(type);
    }
}

Item* EquipComp::GetWeapon() const
{
    Item* result = GetItem(EquipPos::WeaponLeadHand);
    if (!result)
        result = GetItem(EquipPos::WeaponTwoHanded);
    return result;
}

int EquipComp::GetArmor(DamageType damageType, DamagePos pos)
{
    Item* item = nullptr;
    switch (pos)
    {
    case DamagePos::Head:
        item = items_[EquipPos::ArmorHead].get();
        break;
    case DamagePos::Chest:
        item = items_[EquipPos::ArmorChest].get();
        break;
    case DamagePos::Hands:
        item = items_[EquipPos::ArmorHands].get();
        break;
    case DamagePos::Legs:
        item = items_[EquipPos::ArmorLegs].get();
        break;
    case DamagePos::Feet:
        item = items_[EquipPos::ArmorFeet].get();
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

float EquipComp::GetArmorPenetration()
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return 0.0f;
    float value = 0.0f;
    weapon->GetArmorPenetration(value);
    return value;
}

uint32_t EquipComp::GetAttributeValue(uint32_t index)
{
    uint32_t result = 0;
    for (const auto& item : items_)
    {
        item.second->GetAttributeValue(index, result);
    }
    return result;
}

}
}
