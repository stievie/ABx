#pragma once

#include "Variant.h"

class Sessions
{
private:
    std::unordered_map<std::string, std::shared_ptr<Utils::VariantMap>> sessions_;
public:
    Sessions() = default;
    ~Sessions() = default;

    void Add(const std::string& id, std::shared_ptr<Utils::VariantMap> session);
    std::shared_ptr<Utils::VariantMap> Get(const std::string& id);
};

