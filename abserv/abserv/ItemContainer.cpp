#include "stdafx.h"
#include "ItemContainer.h"
#include "Subsystems.h"
#include "ItemFactory.h"

namespace Game {

bool ItemContainer::SetItem(std::unique_ptr<Item>& item, const ItemUpdatedCallback& callback)
{
    if (!item)
        return false;

    if (item->data_.type == AB::Entities::ItemTypeMoney)
    {
        if (!items_[0])
        {
            item->concreteItem_.storagePlace = AB::Entities::StoragePlaceInventory;
            items_[0] = std::move(item);
        }
        else
        {
            if (items_[0]->concreteItem_.count + item->concreteItem_.count > MAX_INVENTOREY_MONEY)
                return false;
            items_[0]->concreteItem_.count += item->concreteItem_.count;
            // Merged -> delete this
            auto factory = GetSubsystem<ItemFactory>();
            factory->DeleteConcrete(item->concreteItem_.uuid);
        }

        callback(items_[0].get());
        return true;
    }

    return AddItem(item, callback);
}

bool ItemContainer::DestroyItem(uint16_t pos)
{
    if (items_.size() > pos && items_[pos])
    {
        auto factory = GetSubsystem<ItemFactory>();
        factory->DeleteItem(items_[pos].get());
        items_[pos].reset();
        return true;
    }
    return false;
}

std::unique_ptr<Item> ItemContainer::RemoveItem(uint16_t pos)
{
    if (items_.size() > pos && items_[pos])
    {
        std::unique_ptr<Item> item = std::move(items_[pos]);
        return item;
    }
    return std::unique_ptr<Item>();
}

Item* ItemContainer::GetItem(uint16_t pos)
{
    if (items_.size() >= pos && items_[pos])
        return items_[pos].get();
    return nullptr;
}

Item* ItemContainer::FindItem(const std::string& uuid)
{
    for (const auto& i : items_)
    {
        if (i && i->data_.uuid.compare(uuid) == 0)
            return i.get();
    }
    return nullptr;
}

bool ItemContainer::StackItem(std::unique_ptr<Item>& item, const ItemUpdatedCallback& callback)
{
    int32_t count = item->concreteItem_.count;
    for (const auto& i : items_)
    {
        if (i && i->data_.index == item->data_.index)
        {
            int32_t space = MAX_INVENTORY_STACK_SIZE - i->concreteItem_.count;
            if (space > 0)
            {
                int32_t added = std::min(space, count);
                i->concreteItem_.count += added;
                count -= added;
                callback(i.get());

                if (count == 0)
                    break;
            }
        }
    }
    if (count == 0)
    {
        // Merged -> delete this
        auto factory = GetSubsystem<ItemFactory>();
        factory->DeleteConcrete(item->concreteItem_.uuid);
        return true;
    }

    // Add remaing as new item
    item->concreteItem_.count = count;
    int16_t p = InsertItem(item);
    if (p != 0)
    {
        callback(items_[p].get());
        return true;
    }

    return false;
}

bool ItemContainer::AddItem(std::unique_ptr<Item>& item, const ItemUpdatedCallback& callback)
{
    // pos = 1-based
    size_t pos = item->concreteItem_.storagePos;
    if (pos == 0)
    {
        // Adding new item to inventory
        if (item->data_.stackAble)
            return StackItem(item, callback);

        // Not stackable
        if (!IsFull())
        {
            int16_t p = InsertItem(item);
            if (p != 0)
            {
                callback(items_[p].get());
                return true;
            }
            return false;
        }
        // Inventory full
        return false;
    }
    else
    {
        // We have a position insert it there
        if (items_.size() > pos)
        {
            if (!items_[pos])
            {
                items_[pos] = std::move(item);
                callback(items_[pos].get());
                return true;
            }
        }
        if (items_.size() < size_)
        {
            if (items_.size() <= pos)
                items_.resize(pos + 1);
            items_[pos] = std::move(item);
            callback(items_[pos].get());
            return true;
        }
        return false;
    }
}

uint16_t ItemContainer::InsertItem(std::unique_ptr<Item>& item)
{
    for (size_t i = 1; i < items_.size(); ++i)
    {
        if (!items_[i])
        {
            items_[i] = std::move(item);
            const auto& _i = items_[i];
            _i->concreteItem_.storagePlace = AB::Entities::StoragePlaceInventory;
            _i->concreteItem_.storagePos = static_cast<uint16_t>(i);
//            _i->concreteItem_.playerUuid = owner_.GetPlayerUuid();
//            _i->concreteItem_.accountUuid = owner_.GetAccountUuid();
            return static_cast<uint16_t>(i);
        }
    }
    // No free slot between -> append it
    if (!IsFull())
    {
        items_.push_back(std::move(item));
        uint16_t pos = static_cast<uint16_t>(items_.size()) - 1;
        const auto& _i = items_[pos];
        _i->concreteItem_.storagePlace = AB::Entities::StoragePlaceInventory;
        _i->concreteItem_.storagePos = pos;
//        _i->concreteItem_.playerUuid = owner_.GetPlayerUuid();
//        _i->concreteItem_.accountUuid = owner_.GetAccountUuid();
        return pos;
    }
    return 0;
}

}
