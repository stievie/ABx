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

#include "Mechanic.h"
#include "Item.h"
#include "ItemsCache.h"
#include <sa/Iteration.h>
#include "Subsystems.h"

namespace Game {

typedef uint16_t ItemPos;

/// Inventory or account chest
class ItemContainer
{
public:
    typedef std::function<void(const Item* const item)> ItemUpdatedCallback;
private:
    size_t stackSize_;
    size_t size_;
    size_t maxMoney_;
    AB::Entities::StoragePlace place_;
    /// All inventory. Index 0 is the money
    std::vector<uint32_t> items_;
    bool AddItem(Item* item, const ItemUpdatedCallback& callback);
    bool StackItem(Item* item, const ItemUpdatedCallback& callback);
    /// Insert in first free slot. Return position
    ItemPos InsertItem(Item* item);
public:
    ItemContainer(size_t stackSize, size_t size, size_t maxMoney, AB::Entities::StoragePlace place) :
        stackSize_(stackSize),
        size_(size),
        maxMoney_(maxMoney),
        place_(place)
    {
        // Money
        items_.resize(1);
    }
    // non-copyable
    ItemContainer(const ItemContainer&) = delete;
    ItemContainer& operator=(const ItemContainer&) = delete;
    ~ItemContainer() = default;

    void InternalSetItem(uint32_t itemId);
    bool SetItem(uint32_t itemId, const ItemUpdatedCallback& callback);
    /// Remove and Destroy (i.e. delete from DB) the item
    bool DestroyItem(ItemPos pos);
    /// Removes the item, does not delete it, e.g. when dropped. Returns the item ID for further anything.
    uint32_t RemoveItem(ItemPos pos);
    Item* GetItem(ItemPos pos);
    Item* FindItem(const std::string& uuid);

    bool IsFull() const { return GetCount() >= size_; }
    void SetSize(size_t value)
    {
        // Can not make smaller
        assert(value + 1u > size_);
        // + 1 because Money doesnt count
        size_ = value + 1;
    }
    size_t GetCount() const
    {
        size_t count = 0;
        for (const auto& i : items_)
        {
            if (i != 0)
                ++count;
        }
        return count;
    }
    template<typename Func>
    void VisitItems(const Func& func);
};

template<typename Func>
inline void ItemContainer::VisitItems(const Func& func)
{
    auto* cache = GetSubsystem<ItemsCache>();
    for (auto& i : items_)
    {
        auto* item = cache->Get(i);
        if (item)
        {
            if (func(*item) != Iteration::Continue)
                break;
        }
    }
}

}
