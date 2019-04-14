#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_ACCOUNTS_KEYS = "account_keys";
static constexpr uint16_t CHEST_SLOT_INCREASE = 10;

enum AccountKeyStatus : uint8_t
{
    KeyStatusUnknown = 0,
    KeyStatusNotActivated = 1,
    KeryStatusReadyForUse = 2,
    KeyStatusBanned = 3
};
enum AccountKeyType : uint8_t
{
    KeyTypeUnknown = 0,
    KeyTypeAccount = 1,
    KeyTypeCharSlot = 2,
    KeyTypeChestSlots = 3,     // Increase count of player inventory by 10
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
        s.value2b(total);
        s.value2b(used);
        s.value1b(status);
        s.text1b(email, Limits::MAX_ACCOUNT_EMAIL);
        s.text1b(description, Limits::MAX_ACCOUNTKEY_DESCRIPTION);
    }

    AccountKeyType type = KeyTypeUnknown;
    uint16_t total = 1;
    uint16_t used = 0;
    AccountKeyStatus status = KeyStatusUnknown;
    std::string email;
    std::string description;
};

}
}
