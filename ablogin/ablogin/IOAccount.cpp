#include "stdafx.h"
#include "IOAccount.h"
#include <abcrypto.hpp>
#include "DataClient.h"
#include <AB/Entities/AccountKey.h>
#include <AB/Entities/AccountKeyAccounts.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/ReservedName.h>
#include <AB/Entities/AccountList.h>
#include <AB/Entities/PlayerItemList.h>
#include "Profiler.h"
#include "Application.h"
#include "Subsystems.h"
#include "UuidUtils.h"

namespace IO {

IOAccount::Result IOAccount::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    if (name.empty())
        return Result::NameExists;
    if (pass.empty())
        return Result::PasswordError;
#if defined(EMAIL_MANDATORY)
    if (pass.empty())
        return Result::EmailError;
#endif
    acc.name = name;
    if (client->Exists(acc))
        return Result::NameExists;

    AB::Entities::AccountKey akey;
    akey.uuid = accKey;
    akey.status = AB::Entities::AccountKeyStatus::KeryStatusReadyForUse;
    akey.type = AB::Entities::AccountKeyType::KeyTypeAccount;
    if (!client->Read(akey))
        return Result::InvalidAccountKey;
    if (akey.used + 1 > akey.total)
        return Result::InvalidAccountKey;

    // Create the account
    char pwhash[61];
    if (bcrypt_newhash(pass.c_str(), 10, pwhash, 61) != 0)
    {
        LOG_ERROR << "bcrypt_newhash() failed" << std::endl;
        return Result::InternalError;
    }
    std::string passwordHash(pwhash, 61);
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    acc.uuid = guid.to_string();
    acc.password = passwordHash;
    acc.email = email;
    acc.type = AB::Entities::AccountType::AccountTypeNormal;
    acc.status = AB::Entities::AccountStatus::AccountStatusActivated;
    acc.creation = Utils::Tick();
    acc.chest_size = AB::Entities::DEFAULT_CHEST_SIZE;
    if (!client->Create(acc))
    {
        LOG_ERROR << "Creating account with name " << name << " failed" << std::endl;
        return Result::InternalError;
    }

    // Bind account to key
    AB::Entities::AccountKeyAccounts aka;
    aka.uuid = akey.uuid;
    aka.accountUuid = acc.uuid;
    if (!client->Create(aka))
    {
        LOG_ERROR << "Creating account - account key failed" << std::endl;
        client->Delete(acc);
        return Result::InternalError;
    }

    // Update account key
    akey.used++;
    if (!client->Update(akey))
    {
        LOG_ERROR << "Updating account key failed" << std::endl;
        client->Delete(aka);
        client->Delete(acc);
        return Result::InternalError;
    }

    AB::Entities::AccountList al;
    client->Invalidate(al);

    return Result::OK;
}

IOAccount::Result IOAccount::AddAccountKey(const std::string& accountUuid, const std::string& pass,
    const std::string& accKey)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
        return Result::InvalidAccount;

    if (bcrypt_checkpass(pass.c_str(), acc.password.c_str()) != 0)
        return Result::InvalidAccount;

    AB::Entities::AccountKey ak;
    ak.uuid = accKey;
    if (!client->Read(ak))
        return Result::InvalidAccountKey;
    if (ak.used + 1 > ak.total)
        return Result::InvalidAccountKey;

    switch (ak.type)
    {
    case AB::Entities::KeyTypeCharSlot:
    {
        acc.charSlots++;
        if (!client->Update(acc))
        {
            LOG_ERROR << "Account update failed " << accountUuid << std::endl;
            return Result::InternalError;
        }
        break;
    }
    case AB::Entities::KeyTypeChestSlots:
    {
        acc.chest_size += AB::Entities::CHEST_SLOT_INCREASE;
        if (!client->Update(acc))
        {
            LOG_ERROR << "Account update failed " << accountUuid << std::endl;
            return Result::InternalError;
        }
        break;
    }
    default:
        return Result::InvalidAccountKey;
    }

    // Bind account to key
    AB::Entities::AccountKeyAccounts aka;
    aka.uuid = ak.uuid;
    aka.accountUuid = acc.uuid;
    if (!client->Create(aka))
    {
        LOG_ERROR << "Creating account - account key failed" << std::endl;
        return Result::InternalError;
    }

    // Update account key
    ak.used++;
    if (!client->Update(ak))
    {
        LOG_ERROR << "Updating account key failed" << std::endl;
        return Result::InternalError;
    }

    return Result::OK;
}

IOAccount::LoginError IOAccount::LoginServerAuth(const std::string& pass,
    AB::Entities::Account& account)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(account))
    {
        LOG_ERROR << "Unable to read account UUID " << account.uuid << std::endl;
        return LoginError::InvalidAccount;
    }
    if (account.status != AB::Entities::AccountStatusActivated)
    {
        LOG_ERROR << "Account not activated UUID " << account.uuid << std::endl;
        return LoginError::InvalidAccount;
    }

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
    if (ch.accountUuid.compare(accountUuid) != 0)
        return false;
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
        const uuids::uuid guid = uuids::uuid_system_generator{}();
        rn.uuid = guid.to_string();
        rn.isReserved = true;
        rn.reservedForAccountUuid = accountUuid;
        rn.name = ch.name;
        rn.expires = Utils::Tick() + NAME_RESERVATION_EXPIRES_MS;
        client->Create(rn);
    }
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
            return forAccountUuid.compare(rn.reservedForAccountUuid) == 0;
        }
        // Exists in table and does not expire, so it's not available.
        return false;
    }
    return true;
}

}
