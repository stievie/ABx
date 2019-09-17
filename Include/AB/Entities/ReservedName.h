#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_RESERVED_NAMES = "reserved_names";

struct ReservedName : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_RESERVED_NAMES;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_RESERVED_NAME);
        s.value1b(isReserved);
        s.text1b(reservedForAccountUuid, Limits::MAX_UUID);
        s.value8b(expires);
    }

    std::string name;
    bool isReserved = false;
    std::string reservedForAccountUuid = EMPTY_GUID;
    timestamp_t expires = 0;
};

}
}
