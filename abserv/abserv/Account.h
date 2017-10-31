#pragma once

#include <vector>
#include <string>
#include <stdint.h>

namespace Auth {

enum AccountType : uint8_t
{
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

class Account
{
public:
    Account() = default;

    std::vector<std::string> characters_;
    std::string name_;
    std::string key_;
    uint32_t id_ = 0;
    AccountType type_ = AccountTypeNormal;
};

}
