#include "stdafx.h"
#include "IOAccount.h"
#include "Database.h"
#include <abcrypto.hpp>
#include "IOGame.h"
#include "Utils.h"
#include "DataClient.h"
#include <AB/Entities/AccountKey.h>
#include <AB/Entities/AccountKeyAccounts.h>
#include <AB/Entities/Character.h>

#include "DebugNew.h"

namespace IO {

IOAccount::Result IOAccount::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
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

#if 0
    Database* db = Database::Instance();
    std::ostringstream query;
    std::shared_ptr<DBResult> result;
    query << "SELECT COUNT(`id`) AS `c` FROM `accounts` WHERE `name` = " << db->EscapeString(name);
    result = db->StoreQuery(query.str());
    if (!result)
        return ResultInternalError;
    if (result->GetUInt("c") != 0)
        return ResultNameExists;

    query.str("");
    query << "SELECT `id`, `account_key`, `used`, `total` FROM `account_keys`"
        << " WHERE `status` = " << static_cast<int>(AccountKeyStatus::ReadyForUse)
        << " AND `key_type` = " << static_cast<int>(AccountKeyType::KeyTypeAccount)
        << " AND `account_key` = " << db->EscapeString(accKey);
    result = db->StoreQuery(query.str());
    if (!result)
        return ResultInvalidAccountKey;

    uint32_t accKeyId = result->GetUInt("id");
    uint32_t used = result->GetUInt("used");
    uint32_t total = result->GetUInt("total");
    if (used + 1 > total)
        return ResultInvalidAccountKey;

    // Create account
    query.str("");
    char pwhash[61];
    if (bcrypt_newhash(pass.c_str(), 10, pwhash, 61) != 0)
    {
        return ResultInternalError;
    }
    std::string passwordHash(pwhash, 61);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return ResultInternalError;

    query << "INSERT INTO `accounts` (`name`, `password`, `email`, `type`, `blocked`, `creation`) VALUES (";
    query << db->EscapeString(name) << ", ";
    query << db->EscapeString(passwordHash) << ", ";
    query << db->EscapeString(email) << ", ";
    query << static_cast<int>(AB::Data::AccountTypeNormal) << ", ";
    query << "0, ";
    query << Utils::AbTick();
    query << ")";
    if (!db->ExecuteQuery(query.str()))
        return ResultInternalError;

    uint64_t accId = db->GetLastInsertId();
    query.str("");
    query << "INSERT INTO `account_account_keys` (`account_id`, `account_keys_id`) VALUES (";
    query << accId << ", ";
    query << accKeyId << ")";
    if (!db->ExecuteQuery(query.str()))
        return ResultInternalError;

    // Update account keys
    query.str("");
    query << "UPDATE `account_keys` SET `used` = `used` + 1 WHERE `id` = " << accKeyId;
    if (!db->ExecuteQuery(query.str()))
        return ResultInternalError;

    // End transaction
    if (!transaction.Commit())
        return ResultInternalError;

    return ResultOK;
#endif
}

IOAccount::Result IOAccount::AddAccountKey(const std::string& name, const std::string& pass,
    const std::string& accKey)
{
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

#if 0
    AB_UNUSED(pass);

    Database* db = Database::Instance();

    std::ostringstream query;
    std::shared_ptr<DBResult> result;
    query << "SELECT `id` FROM `accounts` WHERE `name` = " << db->EscapeString(name);
    result = db->StoreQuery(query.str());
    if (!result)
        return ResultInternalError;
    if (result->Empty())
        return ResultInvalidAccount;
    uint32_t accountId = result->GetUInt("id");

    query.str("");
    query << "SELECT `id`, `account_key`, `used`, `total`, `key_type` FROM `account_keys`"
        << " WHERE `status` = " << static_cast<int>(AccountKeyStatus::ReadyForUse)
        << " AND `account_key` = " << db->EscapeString(accKey);
    result = db->StoreQuery(query.str());
    if (!result)
        return ResultInvalidAccountKey;

    uint32_t accKeyId = result->GetUInt("id");
    uint32_t used = result->GetUInt("used");
    uint32_t total = result->GetUInt("total");
    if (used + 1 > total)
        return ResultInvalidAccountKey;

    AccountKeyType keyType = static_cast<AccountKeyType>(result->GetUInt("key_type"));

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return ResultInternalError;

    switch (keyType)
    {
    case KeyTypeCharSlot:
    {
        query.str("");
        query << "UPDATE `accounts` SET `char_slots` = `char_slots` + 1 WHERE `id` = " << accountId;
        if (!db->ExecuteQuery(query.str()))
            return ResultInternalError;
        break;
    }
    default:
        return ResultInvalidAccountKey;
    }

    // Assign account key to the account
    query.str("");
    query << "INSERT INTO `account_account_keys` (`account_id`, `account_keys_id`) VALUES (";
    query << accountId << ", ";
    query << accKeyId << ")";
    if (!db->ExecuteQuery(query.str()))
        // The key was probably already used for this account
        return ResultInvalidAccountKey;

    // Update account keys
    query.str("");
    query << "UPDATE `account_keys` SET `used` = `used` + 1 WHERE `id` = " << accKeyId;
    if (!db->ExecuteQuery(query.str()))
        return ResultInternalError;

    // End transaction
    if (!transaction.Commit())
        return ResultInternalError;

    return ResultOK;
#endif
}

bool IOAccount::LoginServerAuth(const std::string& name, const std::string& pass, AB::Entities::Account& account)
{
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

#if 0
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `password`, `type`, `char_slots` FROM `accounts` WHERE `name` = " << db->EscapeString(name);
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    std::string dbPass = result->GetString("password");
    if (bcrypt_checkpass(pass.c_str(), dbPass.c_str()) != 0)
        return false;

    account.id_ = result->GetUInt("id");
    account.name_ = result->GetString("name");
    account.type_ = static_cast<AB::Data::AccountType>(result->GetInt("type"));
    account.charSlots_ = result->GetUInt("char_slots");
    account.characters_.clear();

    std::string landingGame = IOGame::GetLandingGame();
    query.str("");
    query << "SELECT `id`, `level`, `experience`, `skillpoints`, `name`, `profession`, `profession2`, `pvp`, ";
    query << "`sex`, `last_map` FROM `players` WHERE `deleted` = 0 AND `account_id` = " << account.id_;
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        std::string lastMap = result->GetString("last_map");
        if (lastMap.empty())
            lastMap = landingGame;
        AB::Data::CharacterData character;
        character.id = result->GetUInt("id");
        character.level = static_cast<uint16_t>(result->GetUInt("level"));
        character.name = result->GetString("name");
        character.prof = result->GetString("profession");
        character.prof2 = result->GetString("profession2");
        character.sex = static_cast<AB::Data::CreatureSex>(result->GetUInt("sex"));
        character.pvp = result->GetUInt("pvp") != 0;
        character.xp = result->GetULong("experience");
        character.skillPoints = result->GetUInt("skillpoints");
        character.lastMap = lastMap;
        account.characters_.push_back(character);
    }
    account.loggedIn_ = true;
    return true;
#endif
}

uuids::uuid IOAccount::GameWorldAuth(const std::string& name, std::string& pass, const std::string& charName)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Account acc;
    acc.name = name;
    if (!client->Read(acc))
        return uuids::uuid();
    if (bcrypt_checkpass(pass.c_str(), acc.password.c_str()) != 0)
        return uuids::uuid();

    AB::Entities::Character ch;
    ch.name = charName;
    if (!client->Read(ch))
        return uuids::uuid();
    if (ch.accountUuid.compare(acc.uuid) != 0)
        return uuids::uuid();

    return uuids::uuid(acc.uuid);
#if 0
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `password` FROM `accounts` WHERE `name` = " << db->EscapeString(name);
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return 0;

    if (bcrypt_checkpass(pass.c_str(), result->GetString("password").c_str()) != 0)
        return false;

    uint32_t accountId = result->GetUInt("id");

    query.str("");
    query << "SELECT `account_id`, `name`, `deleted` FROM `players` WHERE `name` = " <<
        db->EscapeString(charName);
    result = db->StoreQuery(query.str());
    if (!result)
        return 0;
    if (result->GetUInt("account_id") != accountId)
        // Character does not belong to this account
        return 0;
    if (result->GetULong("deleted") != 0)
        // Character was deleted
        return 0;

    return accountId;
#endif
}

bool IOAccount::Save(const AB::Entities::Account& account)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    return client->Update(account);
}

}
