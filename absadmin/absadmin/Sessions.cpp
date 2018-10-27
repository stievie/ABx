#include "stdafx.h"
#include "Sessions.h"

namespace HTTP {

uint32_t Session::sessionLifetime_ = (1000 * 60 * 10);

void Sessions::Add(const std::string& id, std::shared_ptr<Session> session)
{
    sessions_[id] = session;
}

std::shared_ptr<Session> Sessions::Get(const std::string& id)
{
    auto it = sessions_.find(id);
    if (it == sessions_.end())
        return std::shared_ptr<Session>();
    time_t now;
    time(&now);
    if ((*it).second->expires_ < now)
    {
        sessions_.erase(it);
        return std::shared_ptr<Session>();
    }
    return (*it).second;
}

}
