#pragma once

#include "Variant.h"
#include "Utils.h"

namespace HTTP {

struct Session
{
    static uint32_t sessionLifetime_;
    time_t expires_;
    Utils::VariantMap values_;

    Session()
    {
        Touch();
    }

    void Touch()
    {
        time(&expires_);
        struct tm* tm = localtime(&expires_);
        tm->tm_sec += sessionLifetime_ / 1000;
        expires_ = mktime(tm);
    }
};

class Sessions
{
private:
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
public:
    Sessions() = default;
    ~Sessions() = default;

    void Add(const std::string& id, std::shared_ptr<Session> session);
    std::shared_ptr<Session> Get(const std::string& id);

};

}
