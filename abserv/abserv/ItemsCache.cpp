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
#include "ItemsCache.h"
#include <sa/Iteration.h>

namespace Game {

sa::IdGenerator<uint32_t> ItemsCache::itemIds_;

Item* ItemsCache::Get(uint32_t id)
{
    const auto it = itemCache_.find(id);
    if (it == itemCache_.end())
        return nullptr;
    return (*it).second.get();
}

uint32_t ItemsCache::GetConcreteId(const std::string& uuid) const
{
    const auto cIt = concreteIds_.find(uuid);
    if (cIt != concreteIds_.end())
        return (*cIt).second;
    return itemIds_.InvalidId;
}

uint32_t ItemsCache::Add(ea::unique_ptr<Item>&& item)
{
    if (!item)
        return itemIds_.InvalidId;
    if (item->id_ == itemIds_.InvalidId)
        item->id_ = itemIds_.Next();
    const uint32_t id = item->id_;
    const auto cIt = concreteIds_.find(item->concreteItem_.uuid);
    if (cIt == concreteIds_.end())
        concreteIds_.emplace(item->concreteItem_.uuid, id);
    const auto it = itemCache_.find(id);
    if (it == itemCache_.end())
        itemCache_.emplace(id, ea::move(item));

    return id;
}

void ItemsCache::Remove(uint32_t id)
{
    const auto it = itemCache_.find(id);
    if (it != itemCache_.end())
    {
        const std::string& cUuid = (*it).second->concreteItem_.uuid;
        const auto cIt = concreteIds_.find(cUuid);
        if (cIt != concreteIds_.end())
            concreteIds_.erase(cIt);
        itemCache_.erase(it);
    }
}

void ItemsCache::RemoveConcrete(const std::string& uuid)
{
    const auto cIt = concreteIds_.find(uuid);
    if (cIt == concreteIds_.end())
        return;
    uint32_t id = (*cIt).second;
    Remove(id);
}

}
