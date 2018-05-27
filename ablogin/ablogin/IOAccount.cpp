#include "stdafx.h"
#include "IOAccount.h"
#include <abcrypto.hpp>
#include "Utils.h"
#include "DataClient.h"
#include <AB/Entities/AccountKey.h>
#include <AB/Entities/AccountKeyAccounts.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>
#include "Profiler.h"
#include "Application.h"

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
        LOG_ERROR << "bcrypt_newhash() failed" << std::endl;
        return ResultInternalError;
    }
    std::string passwordHash(pwhash, 61);
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    acc.uuid = guid.to_string();
    acc.password = passwordHash;
    acc.email = email;
    acc.type = AB::Entities::AccountType::AccountTypeNormal;
    acc.status = AB::Entities::AccountStatus::AccountStatusActivated;
    acc.creation = Utils::AbTick();
    if (!client->Create(acc))
    {
        LOG_ERROR << "Creating account with name " << name << " failed" << std::endl;
        return ResultInternalError;
    }

    // Bind account to key
    AB::Entities::AccountKeyAccounts aka;
    aka.uuid = akey.uuid;
    aka.accountUuid = acc.uuid;
    if (!client->Create(aka))
    {
        LOG_ERROR << "Creating account - account key failed" << std::endl;
        client->Delete(acc);
        return ResultInternalError;
    }

    // Update account key
    akey.used++;
    if (!client->Update(akey))
    {
        LOG_ERROR << "Updating account key failed" << std::endl;
        client->Delete(aka);
        client->Delete(acc);
        return ResultInternalError;
    }

    return ResultOK;
}

IOAccount::Result IOAccount::AddAccountKey(const std::string& accountUuid, const std::string& pass,
    const std::string& accKey)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
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
        {
            LOG_ERROR << "Account update failed " << accountUuid << std::endl;
            return ResultInternalError;
        }
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
    {
        LOG_ERROR << "Creating account - account key failed" << std::endl;
        return ResultInternalError;
    }

    // Update account key
    ak.used++;
    if (!client->Update(ak))
    {
        LOG_ERROR << "Updating account key failed" << std::endl;
        return ResultInternalError;
    }

    return ResultOK;
}

IOAccount::LoginError IOAccount::LoginServerAuth(const std::string& pass,
    AB::Entities::Account& account)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    if (!client->Read(account))
        return LoginInvalidAccount;
    if (account.status != AB::Entities::AccountStatusActivated)
        return LoginInvalidAccount;

    if (bcrypt_checkpass(pass.c_str(), account.password.c_str()) != 0)
        return LoginPasswordMismatch;

    return LoginOK;
}

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

IOAccount::CreatePlayerResult IOAccount::CreatePlayer(const std::string& accountUuid,
    const std::string& name, const std::string& profUuid, AB::Entities::CharacterSex sex,
    bool isPvp, std::string& uuid)
{
    IO::DataClient* client = Application::Instance->GetDataClient();

    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
        return CreatePlayerResultInvalidAccount;
    if (acc.characterUuids.size() + 1 > acc.charSlots)
        return CreatePlayerResultNoMoreCharSlots;

    AB::Entities::Profession pro;
    pro.uuid = profUuid;
    if (!client->Read(pro))
        return CreatePlayerResultInvalidProfession;

    AB::Entities::Character ch;
    ch.name = name;
    if (client->Exists(ch))
        return CreatePlayerResultNameExists;

    const uuids::uuid guid = uuids::uuid_system_generator{}();
    ch.uuid = guid.to_string();
    ch.name = name;
    ch.profession = pro.abbr;
    ch.professionUuid = pro.uuid;
    ch.sex = sex;
    ch.pvp = isPvp;
    ch.level = 1;
    ch.creation = Utils::AbTick();
    ch.accountUuid = accountUuid;
    if (!client->Create(ch))
    {
        LOG_ERROR << "Create character failed" << std::endl;
        return CreatePlayerResultInternalError;
    }
    // To reload the character list
    client->Invalidate(acc);

    uuid = ch.uuid;
    return CreatePlayerResultOK;
}

bool IOAccount::LoadCharacter(AB::Entities::Character& ch)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    if (!client->Read(ch))
    {
        LOG_ERROR << "Error reading player data" << std::endl;
        return false;
    }
    return true;
}

bool IOAccount::DeletePlayer(const std::string& accountUuid, const std::string& playerUuid)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Character ch;
    ch.uuid = playerUuid;
    if (!client->Read(ch))
        return false;
    if (ch.accountUuid.compare(accountUuid) != 0)
        return false;
    return client->Delete(ch);
}

}
