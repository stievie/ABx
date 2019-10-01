#pragma once

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>

namespace IO {

bool IOAccount_GameWorldAuth(const std::string& accountUuid, const std::string& authToken,
    const std::string& charUuid);
bool IOAccount_AccountLogout(const std::string& uuid);
bool IOAccount_GetAccountInfo(AB::Entities::Account& account, AB::Entities::Character& character);

}
