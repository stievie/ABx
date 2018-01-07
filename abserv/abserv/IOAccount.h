#pragma once

#include "Account.h"

namespace DB {

class IOAccount
{
public:
    enum AccountKeyStatus : uint8_t
    {
        NotActivated = 0,
        ReadyForUse = 1,
        Banned = 2
    };
    enum AccountKeyType : uint8_t
    {
        KeyTypeAccount = 0,
        KeyTypeCharSlot = 1,
    };
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
