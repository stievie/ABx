#include "stdafx.h"
#include "IOAccount.h"
#include <abcrypto.hpp>
#include "Utils.h"
#include "DataClient.h"
#include <AB/Entities/AccountKey.h>
#include <AB/Entities/AccountKeyAccounts.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/ReservedName.h>
#include "Profiler.h"
#include "Application.h"

namespace IO {

IOAccount::Result IOAccount::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Account acc;
    if (name.empty())
        return ResultNameExists;
    if (pass.empty())
        return ResultPasswordError;
#if defined(EMAIL_MANDATORY)
    if (pass.empty())
        return ResultEmailError;
#endif
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
        return LoginError::InvalidAccount;
    if (account.status != AB::Entities::AccountStatusActivated)
        return LoginError::InvalidAccount;

    if (bcrypt_checkpass(pass.c_str(), account.password.c_str()) != 0)
        return LoginError::PasswordMismatch;

    if (account.onlineStatus != AB::Entities::OnlineStatusOffline)
        return LoginError::AlreadyLoggedIn;

    return LoginError::OK;
}

IOAccount::CreatePlayerResult IOAccount::CreatePlayer(const std::string& accountUuid,
    const std::string& name, const std::string& profUuid,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex,
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

    if (!IsNameAvailable(name))
        return CreatePlayerResultNameExists;

    const uuids::uuid guid = uuids::uuid_system_generator{}();
    AB::Entities::Character ch;
    ch.uuid = guid.to_string();
    ch.name = name;
    ch.modelIndex = modelIndex;
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

bool IOAccount::IsNameAvailable(const std::string& name)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Character ch;
    ch.name = name;
    if (client->Exists(ch))
        return false;

    AB::Entities::ReservedName rn;
    rn.name = name;
    rn.isReserved = true;
    return !client->Exists(rn);
}

}
