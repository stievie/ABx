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

void EquipComp::SetItem(ItemPos pos, uint32_t index)
{
    AB::Entities::Item item;
    if (!IO::IOItem::LoadItemByIndex(item, index))
    {
        LOG_ERROR << "Failed to load item with index " << index << std::endl;
        return;
    }
    std::unique_ptr<Item> i = std::make_unique<Item>(item);
    if (i->LoadScript(item.script))
    {
        items_[pos] = std::move(i);
    }
}

Item* EquipComp::GetItem(ItemPos pos) const
{
    auto item = items_.find(pos);
    if (item == items_.end() || !(*item).second)
        return nullptr;

    return (*item).second.get();
}

void EquipComp::RemoveItem(ItemPos pos)
{
    if (!items_[pos])
        return;
    items_[pos].reset();
}

void EquipComp::SetUpgrade(ItemPos pos, ItemUpgrade type, uint32_t index)
{
    if (!items_[pos])
        return;
    items_[pos]->SetUpgrade(type, index);
}

Item* EquipComp::GetWeapon() const
{
    return GetItem(ItemPos::WeaponLeadHand);
}

}
}
