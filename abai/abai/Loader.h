#pragma once

#include "Registry.h"
#include <memory>
#include <string>
#include <CleanupNs.h>

namespace AI {

class Root;
class BevaviorCache;

class Loader
{
protected:
    Registry& registry_;
public:
    explicit Loader(Registry& reg);
    virtual ~Loader();
    virtual std::shared_ptr<Root> LoadFile(const std::string& fileName) = 0;
    virtual std::shared_ptr<Root> LoadString(const std::string& value) = 0;
    // Initialize the cache from an init script
    virtual bool InitChache(const std::string& initScript, BevaviorCache& cache) = 0;
};

}
