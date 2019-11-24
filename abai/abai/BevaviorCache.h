#pragma once

#include <map>
#include <memory>
#include "Root.h"
#include <functional>
#include <sa/Iteration.h>

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
    void Remove(std::shared_ptr<Root> node);
    void VisitBehaviors(const std::function<Iteration(const std::string& name, const Root& root)>& callback) const;
};

}
