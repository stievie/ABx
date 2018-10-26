#include "stdafx.h"
#include "Sessions.h"

void Sessions::Add(const std::string& id, std::shared_ptr<Utils::VariantMap> session)
{
    sessions_[id] = session;
}

std::shared_ptr<Utils::VariantMap> Sessions::Get(const std::string& id)
{
    auto it = sessions_.find(id);
    if (it == sessions_.end())
        return std::shared_ptr<Utils::VariantMap>();
    return (*it).second;
}
