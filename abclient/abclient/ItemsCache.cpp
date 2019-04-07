#include "stdafx.h"
#include "ItemsCache.h"

ItemsCache::ItemsCache(Context* context) :
    Object(context)
{
    // Add a placeholder
    SharedPtr<Item> item = Create();
    item->uuid_ = "00000000-0000-0000-0000-000000000000";
    item->index_ = 0;
    item->name_ = "Placeholder";
    item->type_ = AB::Entities::ItemTypeUnknown;
    item->modelFile_ = "/Objects/Placeholder.xml";
    item->iconFile_ = "/Textures/Icons/placeholder.png";
    item->stackAble_ = false;
    Add(item);
}

ItemsCache::~ItemsCache() = default;

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
        return Get(0);   // Placeholder
    uint32_t index = itemUuids_[uuid];
    return Get(index);
}

SharedPtr<Item> ItemsCache::Get(uint32_t index)
{
    if (!items_.Contains(index))
        return items_[0];
    return items_[index];
}
