#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

constexpr auto KEY_ACCOUNTS_KEYS = "account_keys";

enum AccountKeyStatus : uint8_t
{
    NotActivated = 0,
    ReadyForUse = 1,
    Banned = 2
};
enum AccountKeyType : uint8_t
{
    KeyTypeAccount = 0,
    KeyTypeCharSlot = 1,
};

/// Account key entity. The UUID is the key.
struct AccountKey : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ACCOUNTS_KEYS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value1b(type);
        s.value4b(total);
        s.value4b(used);
        s.value1b(status);
        s.text1b(email, Limits::MAX_ACCOUNT_EMAIL);
        s.text1b(description, Limits::MAX_ACCOUNTKEY_DESCRIPTION);
    }

    AccountKeyType type = KeyTypeAccount;
    uint16_t total = 1;
    uint16_t used = 0;
    AccountKeyStatus status = NotActivated;
    std::string email;
    std::string description;
};

}
}
