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

uint32_t ItemsCache::Add(std::unique_ptr<Item>&& item)
{
    if (!item)
        return itemIds_.InvalidId;
    if (item->id_ == itemIds_.InvalidId)
        item->id_ = itemIds_.Next();
    uint32_t id = item->id_;
    const auto cIt = concreteIds_.find(item->concreteItem_.uuid);
    if (cIt == concreteIds_.end())
        concreteIds_.emplace(item->concreteItem_.uuid, id);
    const auto it = itemCache_.find(id);
    if (it == itemCache_.end())
        itemCache_.emplace(id, std::move(item));

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
