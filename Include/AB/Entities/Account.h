#pragma once

#include "Entity.h"
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

enum AccountType : uint8_t
{
    AccountTyxpeUnknown = 0,
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

struct Account : Entity
{
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
    }

    uint8_t type = AccountTyxpeUnknown;
    bool blocked = false;
    uint64_t creation = 0;
    std::string name;
    std::string password;
    std::string email;
    uint32_t charSlots = 0;
};

}
}
