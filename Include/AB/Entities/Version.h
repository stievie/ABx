#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_VERSIONS = "versions";

struct Version : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_VERSIONS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_VERSION_NAME);
        s.value4b(value);
        s.value1b(isInternal);
    }

    std::string name;
    uint32_t value = 0;
    bool isInternal = false;
};

}
}
