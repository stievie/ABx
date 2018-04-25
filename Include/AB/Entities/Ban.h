#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_BANS = "bans";

enum BanReason : uint8_t
{
    BanReasonUnknown = 0,
    BanReasonScamming,
    BanReasonBotting,
};

struct Ban : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_BANS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value8b(expires);
        s.value8b(added);
        s.value1b(reason);
        s.value1b(active);
        s.text1b(adminUuid, Limits::MAX_UUID);
        s.text1b(comment, Limits::MAX_BAN_COMMENT);
    }

    int64_t expires = 0;
    int64_t added = 0;
    BanReason reason = BanReasonUnknown;
    bool active = false;
    std::string adminUuid;
    std::string comment;
};

}
}
