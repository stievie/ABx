#pragma once

#include "Mechanic.h"
#include "Item.h"
#include "ItemsCache.h"
#include "Iteration.h"
#include "Subsystems.h"

namespace Game {

/// Inventory or account chest
class ItemContainer
{
public:
    typedef std::function<void(const Item* const item)> ItemUpdatedCallback;
private:
    size_t stackSize_;
    size_t size_;
    AB::Entities::StoragePlace place_;
    /// All inventory. Index 0 is the money
    std::vector<uint32_t> items_;
    bool AddItem(Item* item, const ItemUpdatedCallback& callback);
    bool StackItem(Item* item, const ItemUpdatedCallback& callback);
    /// Insert in first free slot. Return position
    uint16_t InsertItem(Item* item);
public:
    ItemContainer(size_t stackSize, size_t size, AB::Entities::StoragePlace place) :
        stackSize_(stackSize),
        size_(size),
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
    bool DestroyItem(uint16_t pos);
    /// Removes the item, does not delete it, e.g. when dropped. Returns the item for further anything.
    /// Since it's a unique_ptr somebody should own it, if it's still needed.
    uint32_t RemoveItem(uint16_t pos);
    Item* GetItem(uint16_t pos);
    Item* FindItem(const std::string& uuid);

    bool IsFull() const { return GetCount() >= size_; }
    void SetSize(uint16_t value)
    {
        // Can not make smaller
        assert(value + 1u > size_);
        // + 1 because Money doesnt count
        size_ = static_cast<size_t>(value) + 1;
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
