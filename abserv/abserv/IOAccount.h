#pragma once

#include "Account.h"

namespace DB {

class IOAccount
{
public:
    IOAccount() = delete;
    static bool LoginServerAuth(const std::string& name, const std::string& pass, Account& account);
    static uint32_t GameWorldAuth(const std::string& name, std::string& pass, const std::string& charName);
    static bool Save(const Account& account);
    static bool Load(Account& account, uint32_t id);
};

}
