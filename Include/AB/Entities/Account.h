#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>
#include <bitsery/traits/vector.h>

namespace AB {
namespace Entities {

constexpr auto KEY_ACCOUNTS = "accounts";
static const uint32_t ACCOUNT_DEF_CHARSLOTS = 6;

enum AccountType : uint8_t
{
    AccountTypeUnknown = 0,
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

enum AccountStatus : uint8_t
{
    AccountStatusUnknown = 0,
    AccountStatusActivated = 1,
    AccountStatusDeleted = 2
};

enum OnlineStatus : uint8_t
{
    OnlineStatusOffline = 0,
    OnlineStatusAway,
    OnlineStatusDoNotDisturb,
    OnlineStatusOnline,
    OnlineStatusInvisible              // Like offline for other users
};

struct Account : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ACCOUNTS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value1b(type);
        s.value1b(status);
        s.value8b(creation);
        s.text1b(name, Limits::MAX_ACCOUNT_NAME);
        s.text1b(password, Limits::MAX_ACCOUNT_PASS);
        s.text1b(email, Limits::MAX_ACCOUNT_EMAIL);
        s.text1b(currentServerUuid, Limits::MAX_UUID);
        s.value4b(charSlots);
        s.text1b(currentCharacterUuid, Limits::MAX_UUID);
        s.value1b(onlineStatus);

        // https://github.com/fraillt/bitsery/blob/master/examples/context_usage.cpp
        // https://github.com/fraillt/bitsery/blob/master/examples/basic_usage.cpp
        s.container(characterUuids, Limits::MAX_ACCOUNT_CHARACTERS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
    }

    AccountType type = AccountTypeUnknown;
    AccountStatus status = AccountStatusUnknown;
    uint64_t creation = 0;
    std::string name;
    std::string password;
    std::string email;
    /// The server currently logged in. Required for cross server chat etc.
    std::string currentServerUuid;
    uint32_t charSlots = ACCOUNT_DEF_CHARSLOTS;
    /// Last or current character
    std::string currentCharacterUuid;
    std::vector<std::string> characterUuids;
    OnlineStatus onlineStatus = OnlineStatusOffline;
};

}
}
