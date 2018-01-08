#pragma once

enum AccountType : uint8_t
{
    AccountTypeNormal = 1,
    AccountTypeTutor = 2,
    AccountTypeSeniorTutor = 3,
    AccountTypeGamemaster = 4,
    AccountTypeGod = 5
};

struct AccountCharacter
{
    uint32_t id;
    uint16_t level;
    std::string name;
    std::string prof;
    std::string prof2;
    std::string lastMap;
};

class Account
{
public:
    Account() = default;

    std::vector<AccountCharacter> characters_;
    std::string name_;
    std::string key_;
    uint32_t id_ = 0;
    uint32_t warnings_ = 0;
    uint32_t charSlots_;
    AccountType type_ = AccountTypeNormal;
    bool loggedIn_ = false;
};
