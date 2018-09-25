#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_IP_BANS = "ip_bans";

struct IpBan : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_IP_BANS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(banUuid, Limits::MAX_UUID);
        s.value4b(ip);
        s.value4b(mask);
    }

    std::string banUuid;
    uint32_t ip = 0;
    uint32_t mask = 0xFFFFFFFF;
};

}
}
