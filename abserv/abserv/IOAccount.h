#pragma once

#include "Account.h"

namespace DB {

class IOAccount
{
public:
    enum CreateAccountResult
    {
        ResultOK,
        ResultNameExists,
        ResultInvalidAccountKey,
        ResultInternalError
    };
    IOAccount() = delete;
    static CreateAccountResult CreateAccount(const std::string& name, const std::string& pass,
        const std::string& email, const std::string& accKey);
    static bool LoginServerAuth(const std::string& name, const std::string& pass, Account& account);
    static uint32_t GameWorldAuth(const std::string& name, std::string& pass, const std::string& charName);
    static bool Save(const Account& account);
};

}
