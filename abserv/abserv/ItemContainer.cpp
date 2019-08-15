#include "stdafx.h"
#include "ItemContainer.h"
#include "ItemFactory.h"

namespace Game {

void ItemContainer::InternalSetItem(uint32_t itemId)
{
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (item)
        items_[item->concreteItem_.storagePos] = itemId;
}

bool ItemContainer::SetItem(uint32_t itemId, const ItemUpdatedCallback& callback)
{
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (!item)
        return false;

    if (item->data_.type == AB::Entities::ItemTypeMoney)
    {
        if (items_[0] == 0)
        {
            item->concreteItem_.storagePlace = place_;
            items_[0] = item->id_;
        }
        else
        {
            auto* currItem = GetItem(0);
            if (currItem->concreteItem_.count + item->concreteItem_.count > MAX_INVENTOREY_MONEY)
                return false;
            currItem->concreteItem_.count += item->concreteItem_.count;
            // Merged -> delete this
            auto* factory = GetSubsystem<ItemFactory>();
            factory->DeleteConcrete(item->concreteItem_.uuid);
        }

        callback(GetItem(0));
        return true;
    }

    return AddItem(item, callback);
}

bool ItemContainer::DestroyItem(uint16_t pos)
{
    if (items_.size() > pos && items_[pos] != 0)
    {
        auto* cache = GetSubsystem<ItemsCache>();
        auto* item = cache->Get(items_[pos]);
        if (!item)
            return false;
        auto* factory = GetSubsystem<ItemFactory>();
        factory->DeleteItem(item);
        items_[pos] = 0;
        return true;
    }
    return false;
}

uint32_t ItemContainer::RemoveItem(uint16_t pos)
{
    if (items_.size() > pos && items_[pos] != 0)
    {
        uint32_t id = items_[pos];
        items_[pos] = 0;
        return id;
    }
    return 0;
}

Item* ItemContainer::GetItem(uint16_t pos)
{
    if (items_.size() >= pos && items_[pos])
    {
        uint32_t id = items_[pos];
        auto* cache = GetSubsystem<ItemsCache>();
        return cache->Get(id);

    }
    return nullptr;
}

Item* ItemContainer::FindItem(const std::string& uuid)
{
    auto* cache = GetSubsystem<ItemsCache>();
    for (uint32_t itemId : items_)
    {
        auto* i = cache->Get(itemId);
        if (!i)
            continue;

        if (i->data_.uuid.compare(uuid) == 0)
            return i;
    }
    return nullptr;
}

bool ItemContainer::StackItem(Item* item, const ItemUpdatedCallback& callback)
{
    int32_t count = static_cast<int32_t>(item->concreteItem_.count);
    auto* cache = GetSubsystem<ItemsCache>();
    for (uint32_t itemId : items_)
    {
        auto* i = cache->Get(itemId);
        if (!i)
            continue;

        if (i->data_.index == item->data_.index)
        {
            int32_t space = static_cast<int32_t>(stackSize_) - static_cast<int32_t>(i->concreteItem_.count);
            if (space > 0)
            {
                int32_t added = std::min(space, count);
                i->concreteItem_.count += added;
                count -= added;
                callback(i);

                if (count == 0)
                    break;
            }
        }
    }

    if (count == 0)
    {
        // Merged -> delete this
        auto* factory = GetSubsystem<ItemFactory>();
        factory->DeleteConcrete(item->concreteItem_.uuid);
        return true;
    }

    // Add remaing as new item
    item->concreteItem_.count = count;
    uint16_t p = InsertItem(item);
    if (p != 0)
    {
        callback(item);
        return true;
    }

    return false;
}

bool ItemContainer::AddItem(Item* item, const ItemUpdatedCallback& callback)
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
            uint16_t p = InsertItem(item);
            if (p != 0)
            {
                callback(item);
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
                items_[pos] = item->id_;
                item->concreteItem_.storagePlace = place_;
                callback(item);
                return true;
            }
        }
        if (items_.size() < size_)
        {
            if (items_.size() <= pos)
                items_.resize(pos + 1);
            items_[pos] = item->id_;
            item->concreteItem_.storagePlace = place_;
            callback(item);
            return true;
        }
        return false;
    }
}

uint16_t ItemContainer::InsertItem(Item* item)
{
    for (size_t i = 1; i < items_.size(); ++i)
    {
        if (items_[i] == 0)
        {
            items_[i] = item->id_;
            item->concreteItem_.storagePlace = place_;
            item->concreteItem_.storagePos = static_cast<uint16_t>(i);
            return static_cast<uint16_t>(i);
        }
    }
    // No free slot between -> append it
    if (!IsFull())
    {
        items_.push_back(item->id_);
        uint16_t pos = static_cast<uint16_t>(items_.size()) - 1;
        item->concreteItem_.storagePlace = place_;
        item->concreteItem_.storagePos = pos;
        return pos;
    }
    return 0;
}

}
