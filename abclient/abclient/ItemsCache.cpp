#include "stdafx.h"
#include "ItemsCache.h"

ItemsCache::ItemsCache(Context* context) :
    Object(context)
{
}

ItemsCache::~ItemsCache()
{
}

SharedPtr<Item> ItemsCache::Create()
{
    return SharedPtr<Item>(new Item(context_));
}

bool ItemsCache::Add(SharedPtr<Item> item)
{
    if (items_.Contains(item->index_))
        return false;
    items_[item->index_] = item;
    itemUuids_[item->uuid_] = item->index_;
    return true;
}

SharedPtr<Item> ItemsCache::Get(const String& uuid)
{
    if (!itemUuids_.Contains(uuid))
        return SharedPtr<Item>();
    uint32_t index = itemUuids_[uuid];
    return Get(index);
}

SharedPtr<Item> ItemsCache::Get(uint32_t index)
{
    if (!items_.Contains(index))
        return SharedPtr<Item>();
    return items_[index];
}
