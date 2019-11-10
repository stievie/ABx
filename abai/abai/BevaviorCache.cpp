#include "stdafx.h"
#include "BevaviorCache.h"

namespace AI {

std::shared_ptr<Root> BevaviorCache::Get(const std::string& name) const
{
    const auto it = cache_.find(name);
    if (it == cache_.end())
        return std::shared_ptr<Root>();
    return (*it).second;
}

void BevaviorCache::Add(std::shared_ptr<Root> node)
{
    if (!node)
        return;
    cache_.emplace(node->GetName(), node);
}

}
