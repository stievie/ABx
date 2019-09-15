#pragma once

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>

namespace IO {

class IOAccount
{
public:
    IOAccount() = delete;
    static bool GameWorldAuth(const std::string& accountUuid, const std::string& authToken,
        const std::string& charUuid);
    static bool Save(const AB::Entities::Account& account);
    static bool AccountLogout(const std::string& uuid);
    static bool GetAccountInfo(AB::Entities::Account& account, AB::Entities::Character& character);
};

}
