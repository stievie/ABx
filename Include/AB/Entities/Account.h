#pragma once

#include "Entity.h"
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>

using bitsery::ext::BaseClass;
using bitsery::ext::VirtualBaseClass;

namespace AB {
namespace Entities {

namespace Limits {
static constexpr int MAX_ACCOUNT_NAME = 32;
static constexpr int MAX_ACCOUNT_PASS = 60;
}

enum AccountType : uint8_t
{
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

struct Account : Entity
{
    uint8_t type;
    bool blocked;
    uint64_t creation;
    std::string name;
    std::string password;
    std::string email;
    uint32_t charSlots;
};

template <typename S>
void serialize(S& s, Account& o)
{
    s.ext(o, BaseClass<Entity>{});
    s.value1b(o.type);
    s.text1b(o.name, Limits::MAX_ACCOUNT_NAME);
    s.text1b(o.password, Limits::MAX_ACCOUNT_PASS);
}

}
}
