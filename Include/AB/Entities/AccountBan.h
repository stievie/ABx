#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_ACCOUNTS_BANS = "account_bans";

/// Account ban entity.
struct AccountBan : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ACCOUNTS_BANS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(banUuid, Limits::MAX_UUID);
        s.text1b(accountUuid, Limits::MAX_UUID);
    }

    std::string banUuid;
    std::string accountUuid;
};

}
}
