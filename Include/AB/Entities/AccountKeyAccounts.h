#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_ACCOUNTKEYACCOUNTS = "account_account_keys";

/// Account key entity. The UUID is the key.
struct AccountKeyAccounts : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ACCOUNTKEYACCOUNTS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(accountUuid, Limits::MAX_UUID);
    }

    std::string accountUuid;
};

}
}
