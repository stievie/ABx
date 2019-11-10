#pragma once

#include <map>
#include <memory>
#include "Root.h"

namespace AI {

class BevaviorCache
{
private:
    std::map<std::string, std::shared_ptr<Root>> cache_;
public:
    BevaviorCache() = default;
    ~BevaviorCache() = default;

    std::shared_ptr<Root> Get(const std::string& name) const;
    void Add(std::shared_ptr<Root> node);
};

}
