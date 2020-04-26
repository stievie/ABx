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

    if (item->data_.type == AB::Entities::ItemType::Money)
    {
        if (items_[0] == 0)
        {
            item->concreteItem_.storagePlace = place_;
            items_[0] = item->id_;
        }
        else
        {
            auto* currItem = GetItem(0);
            if (Utils::WouldExceed(currItem->concreteItem_.count, item->concreteItem_.count, static_cast<uint32_t>(maxMoney_)))
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

bool ItemContainer::DestroyItem(ItemPos pos)
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

uint32_t ItemContainer::RemoveItem(ItemPos pos)
{
    if (items_.size() > pos && items_[pos] != 0)
    {
        uint32_t id = items_[pos];
        items_[pos] = 0;
        return id;
    }
    return 0;
}

Item* ItemContainer::GetItem(ItemPos pos) const
{
    if (items_.size() >= pos && items_[pos])
    {
        uint32_t id = items_[pos];
        auto* cache = GetSubsystem<ItemsCache>();
        return cache->Get(id);

    }
    return nullptr;
}

uint32_t ItemContainer::GetMoney() const
{
    auto* item = GetItem(0);
    if (!item)
        return 0;
    return item->concreteItem_.count;
}

size_t ItemContainer::GetMoneyFreeSpace() const
{
    return maxMoney_ - GetMoney();
}

bool ItemContainer::CheckCapacity(uint32_t money, size_t itemCount)
{
    if (GetFreeSpace() < itemCount)
        return false;
    if (GetMoneyFreeSpace() < money)
        return false;
    return true;
}

Item* ItemContainer::FindItem(const std::string& uuid)
{
    auto* cache = GetSubsystem<ItemsCache>();
    for (uint32_t itemId : items_)
    {
        auto* i = cache->Get(itemId);
        if (!i)
            continue;

        if (Utils::Uuid::IsEqual(i->data_.uuid, uuid))
            return i;
    }
    return nullptr;
}

bool ItemContainer::StackItem(Item* item, const ItemUpdatedCallback& callback)
{
    uint32_t count = item->concreteItem_.count;
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
                uint32_t added = std::min(static_cast<uint32_t>(space), count);
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

    // Add remaining as new item
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

ItemPos ItemContainer::InsertItem(Item* item)
{
    for (size_t i = 1; i < items_.size(); ++i)
    {
        if (items_[i] == 0)
        {
            items_[i] = item->id_;
            item->concreteItem_.storagePlace = place_;
            item->concreteItem_.storagePos = static_cast<uint16_t>(i);
            return static_cast<ItemPos>(i);
        }
    }
    // No free slot between -> append it
    if (!IsFull())
    {
        items_.push_back(item->id_);
        ItemPos pos = static_cast<ItemPos>(items_.size()) - 1;
        item->concreteItem_.storagePlace = place_;
        item->concreteItem_.storagePos = pos;
        return pos;
    }
    return 0;
}

}
