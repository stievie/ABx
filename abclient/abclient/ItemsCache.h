#pragma once

#include "Item.h"

class ItemsCache : public Object
{
    URHO3D_OBJECT(ItemsCache, Object)
private:
    HashMap<uint32_t, SharedPtr<Item>> items_;
    HashMap<String, uint32_t> itemUuids_;
public:
    ItemsCache(Context* context);
    ~ItemsCache() override;

    SharedPtr<Item> Create();
    bool Add(SharedPtr<Item> item);
    SharedPtr<Item> Get(const String& uuid);
    SharedPtr<Item> Get(uint32_t index);
};

