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

void BevaviorCache::Remove(std::shared_ptr<Root> node)
{
    if (!node)
        return;
    auto it = cache_.find(node->GetName());
    if (it == cache_.end())
        return;
    cache_.erase(it);
}

void BevaviorCache::VisitBehaviors(const std::function<Iteration(const std::string& name, const Root& root)>& callback) const
{
    for (const auto& i : cache_)
    {
        if (callback(i.first, *i.second) != Iteration::Continue)
            break;
    }
}

}
