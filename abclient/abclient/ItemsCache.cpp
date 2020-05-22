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


#include "ItemsCache.h"

ItemsCache::ItemsCache(Context* context) :
    Object(context)
{
    // Add a placeholder
    SharedPtr<Item> item = Create();
    item->uuid_ = "00000000-0000-0000-0000-000000000000";
    item->index_ = 0;
    item->name_ = "Placeholder";
    item->type_ = AB::Entities::ItemType::Unknown;
    item->objectFile_ = "/Objects/Placeholder.xml";
    item->iconFile_ = "/Textures/Icons/placeholder.png";
    item->stackAble_ = false;
    Add(item);
}

ItemsCache::~ItemsCache() = default;

SharedPtr<Item> ItemsCache::Create()
{
    return MakeShared<Item>(context_);
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
