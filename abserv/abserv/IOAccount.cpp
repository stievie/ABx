#include "stdafx.h"
#include "IOAccount.h"
#include "Database.h"
#include <abcrypto.hpp>
#include "IOGame.h"
#include "Utils.h"

#include "DebugNew.h"

namespace DB {

IOAccount::Result IOAccount::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
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
}

IOAccount::Result IOAccount::AddAccountKey(const std::string& name, const std::string& pass,
    const std::string& accKey)
{
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
}

bool IOAccount::LoginServerAuth(const std::string& name, const std::string& pass, AB::Data::AccountData& account)
{
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
}

uint32_t IOAccount::GameWorldAuth(const std::string& name, std::string& pass, const std::string& charName)
{
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
}

bool IOAccount::Save(const AB::Data::AccountData& account)
{
    return true;
    /*
    Database* db = Database::Instance();
    DBQuery query;
    query << "UPDATE `accounts` SET `warnings` = " << account.warnings_ << " WHERE `id` = " << account.id_;
    return db->ExecuteQuery(query);
    */
}

}
