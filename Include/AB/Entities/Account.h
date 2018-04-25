#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>
#include <bitsery/traits/vector.h>

namespace AB {
namespace Entities {

constexpr auto KEY_ACCOUNTS = "accounts";

enum AccountType : uint8_t
{
    AccountTypeUnknown = 0,
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
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
        s.value1b(blocked);
        s.value8b(creation);
        s.text1b(name, Limits::MAX_ACCOUNT_NAME);
        s.text1b(password, Limits::MAX_ACCOUNT_PASS);
        s.text1b(email, Limits::MAX_ACCOUNT_EMAIL);
        s.value4b(charSlots);
        s.text1b(lastCharacterUuid, Limits::MAX_UUID);

        // https://github.com/fraillt/bitsery/blob/master/examples/context_usage.cpp
        // https://github.com/fraillt/bitsery/blob/master/examples/basic_usage.cpp
        s.container(characterUuids, Limits::MAX_ACCOUNT_CHARACTERS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
    }

    AccountType type = AccountTypeUnknown;
    bool blocked = false;
    uint64_t creation = 0;
    std::string name;
    std::string password;
    std::string email;
    uint32_t charSlots = 0;
    std::string lastCharacterUuid;
    std::vector<std::string> characterUuids;
};

}
}
