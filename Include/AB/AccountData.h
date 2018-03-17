#pragma once

#include <vector>
#include <AB/CharacterData.h>

namespace AB {
namespace Data {

enum AccountType : uint8_t
{
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

class AccountData
{
public:
    AccountData() = default;

    CharacterList characters_;
    std::string name_;
    std::string key_;
    uint32_t id_ = 0;
    uint32_t warnings_ = 0;
    uint32_t charSlots_;
    AccountType type_ = AccountTypeNormal;
    bool loggedIn_ = false;
};

}
}
