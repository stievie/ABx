#pragma once

#include <AB/Entities/Account.h>

namespace IO {

class IOAccount
{
public:
    IOAccount() = delete;
    static bool GameWorldAuth(const std::string& accountUuid, const std::string& pass,
        const std::string& charUuid);
    static bool Save(const AB::Entities::Account& account);
    static bool AccountLogout(const std::string& uuid);
};

}
