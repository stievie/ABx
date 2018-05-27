#include "stdafx.h"
#include "IOAccount.h"
#include <abcrypto.hpp>
#include "IOGame.h"
#include "Utils.h"
#include "DataClient.h"
#include <AB/Entities/AccountKey.h>
#include <AB/Entities/AccountKeyAccounts.h>
#include <AB/Entities/Character.h>
#include "Profiler.h"

#include "DebugNew.h"

namespace IO {

bool IOAccount::GameWorldAuth(const std::string& accountUuid, const std::string& pass,
    const std::string& charUuid)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
    {
        // At this point the account must exist.
        LOG_ERROR << "Error reading account " << accountUuid << std::endl;
        return false;
    }
    if (bcrypt_checkpass(pass.c_str(), acc.password.c_str()) != 0)
        return false;

    AB::Entities::Character ch;
    ch.uuid = charUuid;
    if (!client->Read(ch))
    {
        LOG_ERROR << "Error reading character " << charUuid << std::endl;
        return false;
    }
    if (ch.accountUuid.compare(acc.uuid) != 0)
    {
        LOG_ERROR << "Character " << charUuid << " does not belong to account " << accountUuid << std::endl;
        return false;
    }

    return true;
}

bool IOAccount::Save(const AB::Entities::Account& account)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    return client->Update(account);
}

bool IOAccount::AccountLogout(const std::string& uuid)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Account acc;
    acc.uuid = uuid;
    if (!client->Read(acc))
    {
        LOG_ERROR << "Error reading account " << uuid << std::endl;
        return false;
    }
    LOG_INFO << acc.name << " logged out" << std::endl;
    acc.onlineStatus = AB::Entities::OnlineStatus::OnlineStatusOffline;
    return client->Update(acc);
}

}
