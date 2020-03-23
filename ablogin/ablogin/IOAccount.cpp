/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "IOAccount.h"
#include "Application.h"
#include <AB/Entities/AccountKey.h>
#include <AB/Entities/AccountKeyAccounts.h>
#include <AB/Entities/AccountList.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/PlayerItemList.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/ReservedName.h>
#include <abcrypto.hpp>
#include <abscommon/DataClient.h>
#include <abscommon/Profiler.h>
#include <abscommon/Subsystems.h>
#include <abscommon/UuidUtils.h>

namespace IO {

IOAccount::CreateAccountResult IOAccount::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    if (name.empty())
        return CreateAccountResult::NameExists;
    if (pass.empty())
        return CreateAccountResult::PasswordError;
#if defined(EMAIL_MANDATORY)
    if (email.empty())
        return Result::EmailError;
#endif
    acc.name = name;
    if (client->Exists(acc))
        return CreateAccountResult::NameExists;

    AB::Entities::AccountKey akey;
    akey.uuid = accKey;
    akey.status = AB::Entities::AccountKeyStatus::KeryStatusReadyForUse;
    akey.type = AB::Entities::AccountKeyType::KeyTypeAccount;
    if (!client->Read(akey))
        return CreateAccountResult::InvalidAccountKey;
    if (akey.used + 1 > akey.total)
        return CreateAccountResult::InvalidAccountKey;

    // Create the account
    char pwhash[61];
    if (bcrypt_newhash(pass.c_str(), 10, pwhash, 61) != 0)
    {
        LOG_ERROR << "bcrypt_newhash() failed" << std::endl;
        return CreateAccountResult::InternalError;
    }
    std::string passwordHash(pwhash, 61);
    acc.uuid = Utils::Uuid::New();
    acc.password = passwordHash;
    acc.email = email;
    acc.authToken = Utils::Uuid::New();
    acc.authTokenExpiry = Utils::Tick() + Auth::AUTH_TOKEN_EXPIRES_IN;
    acc.type = AB::Entities::AccountType::AccountTypeNormal;
    acc.status = AB::Entities::AccountStatus::AccountStatusActivated;
    acc.creation = Utils::Tick();
    acc.chest_size = AB::Entities::DEFAULT_CHEST_SIZE;
    if (!client->Create(acc))
    {
        LOG_ERROR << "Creating account with name " << name << " failed" << std::endl;
        return CreateAccountResult::InternalError;
    }

    // Bind account to key
    AB::Entities::AccountKeyAccounts aka;
    aka.uuid = akey.uuid;
    aka.accountUuid = acc.uuid;
    if (!client->Create(aka))
    {
        LOG_ERROR << "Creating account - account key failed" << std::endl;
        client->Delete(acc);
        return CreateAccountResult::InternalError;
    }

    // Update account key
    akey.used++;
    if (!client->Update(akey))
    {
        LOG_ERROR << "Updating account key failed" << std::endl;
        client->Delete(aka);
        client->Delete(acc);
        return CreateAccountResult::InternalError;
    }

    AB::Entities::AccountList al;
    client->Invalidate(al);

    return CreateAccountResult::OK;
}

IOAccount::CreateAccountResult IOAccount::AddAccountKey(AB::Entities::Account& account,
    const std::string& accKey)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();

    AB::Entities::AccountKey ak;
    ak.uuid = accKey;
    if (!client->Read(ak))
        return CreateAccountResult::InvalidAccountKey;
    if (ak.used + 1 > ak.total)
        return CreateAccountResult::InvalidAccountKey;

    // Bind account to key
    AB::Entities::AccountKeyAccounts aka;
    aka.uuid = ak.uuid;
    aka.accountUuid = account.uuid;
    if (!client->Create(aka))
    {
        LOG_ERROR << "Creating account - account key failed" << std::endl;
        return CreateAccountResult::AlreadyAdded;
    }

    // Update account key
    ak.used++;
    if (!client->Update(ak))
    {
        LOG_ERROR << "Updating account key failed" << std::endl;
        return CreateAccountResult::InternalError;
    }

    switch (ak.type)
    {
    case AB::Entities::KeyTypeCharSlot:
    {
        account.charSlots++;
        if (!client->Update(account))
        {
            LOG_ERROR << "Account update failed " << account.uuid << std::endl;
            return CreateAccountResult::InternalError;
        }
        break;
    }
    case AB::Entities::KeyTypeChestSlots:
    {
        account.chest_size += AB::Entities::CHEST_SLOT_INCREASE;
        if (!client->Update(account))
        {
            LOG_ERROR << "Account update failed " << account.uuid << std::endl;
            return CreateAccountResult::InternalError;
        }
        break;
    }
    default:
        return CreateAccountResult::InvalidAccountKey;
    }

    return CreateAccountResult::OK;
}

IOAccount::PasswordAuthResult IOAccount::PasswordAuth(const std::string& pass,
    AB::Entities::Account& account)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(account))
    {
        LOG_ERROR << "Unable to read account UUID " << account.uuid << " name " << account.name << std::endl;
        return PasswordAuthResult::InvalidAccount;
    }
    if (account.status != AB::Entities::AccountStatusActivated)
    {
        LOG_ERROR << "Account not activated UUID " << account.uuid << std::endl;
        return PasswordAuthResult::InvalidAccount;
    }

    if (bcrypt_checkpass(pass.c_str(), account.password.c_str()) != 0)
        return PasswordAuthResult::PasswordMismatch;

    if (account.onlineStatus != AB::Entities::OnlineStatusOffline)
        return PasswordAuthResult::AlreadyLoggedIn;

    account.authToken = Utils::Uuid::New();
    account.authTokenExpiry = Utils::Tick() + Auth::AUTH_TOKEN_EXPIRES_IN;
    if (!client->Update(account))
        return PasswordAuthResult::InternalError;

    return PasswordAuthResult::OK;
}

bool IOAccount::TokenAuth(const std::string& token, AB::Entities::Account& account)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(account))
    {
        LOG_ERROR << "Unable to read account UUID " << account.uuid << " name " << account.name << std::endl;
        return false;
    }
    if (!Utils::Uuid::IsEqual(account.authToken, token))
        return false;
    if (Utils::IsExpired(account.authTokenExpiry))
        return false;

    // Refresh token
    account.authTokenExpiry = Utils::Tick() + Auth::AUTH_TOKEN_EXPIRES_IN;
    client->Update(account);

    return true;
}

IOAccount::CreatePlayerResult IOAccount::CreatePlayer(const std::string& accountUuid,
    const std::string& name, const std::string& profUuid,
    uint32_t modelIndex,
    AB::Entities::CharacterSex sex,
    bool isPvp, std::string& uuid)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();

    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
        return CreatePlayerResult::InvalidAccount;
    if (acc.characterUuids.size() + 1 > acc.charSlots)
        return CreatePlayerResult::NoMoreCharSlots;

    AB::Entities::Profession pro;
    pro.uuid = profUuid;
    if (!client->Read(pro))
        return CreatePlayerResult::InvalidProfession;

    if (!IsNameAvailable(name, accountUuid))
        return CreatePlayerResult::NameExists;
    if (name.find_first_of(RESTRICTED_NAME_CHARS, 0) != std::string::npos)
        return CreatePlayerResult::InvalidName;

    AB::Entities::Character ch;
    ch.uuid = Utils::Uuid::New();
    ch.name = name;
    ch.modelIndex = modelIndex;
    ch.profession = pro.abbr;
    ch.professionUuid = pro.uuid;
    ch.sex = sex;
    ch.pvp = isPvp;
    ch.level = 1;
    ch.creation = Utils::Tick();
    ch.inventory_size = AB::Entities::DEFAULT_INVENTORY_SIZE;
    ch.accountUuid = accountUuid;
    if (!client->Create(ch))
    {
        LOG_ERROR << "Create character failed" << std::endl;
        return CreatePlayerResult::InternalError;
    }
    // To reload the character list
    client->Invalidate(acc);

    // If in reserved names table we must delete it now
    AB::Entities::ReservedName rn;
    rn.name = ch.name;
    client->DeleteIfExists(rn);

    uuid = ch.uuid;
    return CreatePlayerResult::OK;
}

bool IOAccount::LoadCharacter(AB::Entities::Character& ch)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(ch))
    {
        LOG_ERROR << "Error reading player data" << std::endl;
        return false;
    }
    // HACK: Reset party ID here :(
    // Otherwise people may find themselves in the same party days after.
    ch.partyUuid = Utils::Uuid::EMPTY_UUID;
    client->Update(ch);
    return true;
}

bool IOAccount::DeletePlayer(const std::string& accountUuid, const std::string& playerUuid)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Character ch;
    ch.uuid = playerUuid;
    if (!client->Read(ch))
        return false;
    if (!Utils::Uuid::IsEqual(ch.accountUuid, accountUuid))
        return false;
    AB::Entities::Account account;
    account.uuid = accountUuid;
    if (!client->Read(account))
    {
        LOG_ERROR << "Error reading account " << accountUuid << std::endl;
        return false;
    }

    bool succ = client->Delete(ch);
    if (!succ)
    {
        LOG_ERROR << "Error deleting player with UUID " << playerUuid << std::endl;
        return false;
    }

    // Everything that belongs to the Player (not the Account) should be deleted too.
    // TODO: Check if there is more to delete.
    const auto deleteItems = [&](const std::vector<std::string>& items)
    {
        for (const auto& uuid : items)
        {
            AB::Entities::ConcreteItem item;
            item.uuid = uuid;
            if (!client->Delete(item))
                LOG_WARNING << "Error deleting concrete item with UUID " << uuid << std::endl;
        }
    };

    AB::Entities::EquippedItems equip;
    equip.uuid = playerUuid;
    succ = client->Read(equip);
    if (succ)
        deleteItems(equip.itemUuids);
    else
        LOG_WARNING << "Error reading equipment for player with UUID " << playerUuid << std::endl;

    AB::Entities::InventoryItems inv;
    inv.uuid = playerUuid;
    succ = client->Read(inv);
    if (succ)
        deleteItems(inv.itemUuids);
    else
        LOG_ERROR << "Error reading inventory for player with UUID " << playerUuid << std::endl;

    if (succ)
    {
        // Reserve the character name for some time for this user
        AB::Entities::ReservedName rn;
        rn.name = ch.name;
        client->DeleteIfExists(rn);
        client->Invalidate(rn);
        rn.uuid = Utils::Uuid::New();
        rn.isReserved = true;
        rn.reservedForAccountUuid = accountUuid;
        rn.name = ch.name;
        rn.expires = Utils::Tick() + NAME_RESERVATION_EXPIRES_MS;
        client->Create(rn);
    }
    // Update character list
    client->Invalidate(ch);
    client->Invalidate(account);
    return succ;
}

bool IOAccount::IsNameAvailable(const std::string& name, const std::string& forAccountUuid)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Character ch;
    ch.name = name;
    // Check if player with the same name exists
    if (client->Exists(ch))
        return false;

    AB::Entities::ReservedName rn;
    rn.name = name;
    rn.isReserved = true;
    if (client->Read(rn))
    {
        // Temporarily reserved for an account
        if (rn.expires != 0)
        {
            if (rn.expires < Utils::Tick())
            {
                // Expired -> Delete it
                client->Delete(rn);
                return true;
            }
            // Not expired yet
            return Utils::Uuid::IsEqual(forAccountUuid, rn.reservedForAccountUuid);
        }
        // Exists in table and does not expire, so it's not available.
        return false;
    }
    return true;
}

}
