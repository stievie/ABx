#pragma once

#include <AB/Entities/Account.h>

namespace IO {

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
    enum Result
    {
        ResultOK,
        ResultNameExists,
        ResultInvalidAccountKey,
        ResultInvalidAccount,
        ResultInternalError
    };
    IOAccount() = delete;
    static Result CreateAccount(const std::string& name, const std::string& pass,
        const std::string& email, const std::string& accKey);
    static Result AddAccountKey(const std::string& name, const std::string& pass,
        const std::string& accKey);
    static bool LoginServerAuth(const std::string& name, const std::string& pass, AB::Entities::Account& account);
    static uuids::uuid GameWorldAuth(const std::string& name, const std::string& pass, const std::string& charUuid);
    static bool Save(const AB::Entities::Account& account);
};

}
