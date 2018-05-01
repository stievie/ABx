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

IOAccount::Result IOAccount::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Account acc;
    acc.name = name;
    if (client->Exists(acc))
        return ResultNameExists;

    AB::Entities::AccountKey akey;
    akey.uuid = accKey;
    akey.status = AB::Entities::AccountKeyStatus::ReadyForUse;
    akey.type = AB::Entities::AccountKeyType::KeyTypeAccount;
    if (!client->Read(akey))
        return ResultInvalidAccountKey;
    if (akey.used + 1 > akey.total)
        return ResultInvalidAccountKey;

    // Create the account
    char pwhash[61];
    if (bcrypt_newhash(pass.c_str(), 10, pwhash, 61) != 0)
    {
        return ResultInternalError;
    }
    std::string passwordHash(pwhash, 61);
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    acc.uuid = guid.to_string();
    acc.password = passwordHash;
    acc.email = email;
    acc.type = AB::Entities::AccountType::AccountTypeNormal;
    acc.creation = Utils::AbTick();
    if (!client->Create(acc))
        return ResultInternalError;

    // Bind account to key
    AB::Entities::AccountKeyAccounts aka;
    aka.uuid = akey.uuid;
    aka.accountUuid = acc.uuid;
    if (!client->Create(aka))
        return ResultInternalError;

    // Update account key
    akey.used++;
    if (!client->Update(akey))
        return ResultInternalError;

    return ResultOK;
}

IOAccount::Result IOAccount::AddAccountKey(const std::string& name, const std::string& pass,
    const std::string& accKey)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Account acc;
    acc.name = name;
    if (!client->Read(acc))
        return ResultInvalidAccount;

    if (bcrypt_checkpass(pass.c_str(), acc.password.c_str()) != 0)
        return ResultInvalidAccount;

    AB::Entities::AccountKey ak;
    ak.uuid = accKey;
    if (!client->Read(ak))
        return ResultInvalidAccountKey;
    if (ak.used + 1 > ak.total)
        return ResultInvalidAccountKey;

    switch (ak.type)
    {
    case AB::Entities::KeyTypeCharSlot:
    {
        acc.charSlots++;
        if (!client->Update(acc))
            return ResultInternalError;
        break;
    }
    default:
        return ResultInvalidAccountKey;
    }

    // Bind account to key
    AB::Entities::AccountKeyAccounts aka;
    aka.uuid = ak.uuid;
    aka.accountUuid = acc.uuid;
    if (!client->Create(aka))
        return ResultInternalError;

    // Update account key
    ak.used++;
    if (!client->Update(ak))
        return ResultInternalError;

    return ResultOK;
}

bool IOAccount::LoginServerAuth(const std::string& name, const std::string& pass, AB::Entities::Account& account)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    account.name = name;
    if (!client->Read(account))
        return false;

    if (bcrypt_checkpass(pass.c_str(), account.password.c_str()) != 0)
        return false;

    const std::string landingGame = IOGame::GetLandingGame();

    account.onlineStatus = AB::Entities::OnlineStatus::OnlineStatusOnline;
    client->Update(account);

    return true;
}

uuids::uuid IOAccount::GameWorldAuth(const std::string& name, const std::string& pass, const std::string& charUuid)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Account acc;
    acc.name = name;
    if (!client->Read(acc))
        return uuids::uuid();
    if (bcrypt_checkpass(pass.c_str(), acc.password.c_str()) != 0)
        return uuids::uuid();

    AB::Entities::Character ch;
    ch.uuid = charUuid;
    if (!client->Read(ch))
        return uuids::uuid();
    if (ch.accountUuid.compare(acc.uuid) != 0)
        return uuids::uuid();

    return uuids::uuid(acc.uuid);
}

bool IOAccount::Save(const AB::Entities::Account& account)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    return client->Update(account);
}

}
