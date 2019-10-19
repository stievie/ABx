#pragma once

#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/GuildMembers.h>

namespace IO {
namespace IOAccount {

bool GameWorldAuth(const std::string& accountUuid, const std::string& authToken,
    const std::string& charUuid);
bool AccountLogout(const std::string& uuid);
bool GetAccountInfo(AB::Entities::Account& account, AB::Entities::Character& character);
bool GetGuildMemberInfo(const AB::Entities::Account& account, AB::Entities::GuildMember& g);

}
}
