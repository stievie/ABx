#include "stdafx.h"
#include "IOAccount.h"
#include "Database.h"
#include <abcrypto.hpp>
#include "IOGame.h"
#include "Utils.h"

#include "DebugNew.h"

namespace DB {

IOAccount::CreateAccountResult IOAccount::CreateAccount(const std::string& name, const std::string& pass,
    const std::string& email, const std::string& accKey)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    std::shared_ptr<DBResult> result;
    query << "SELECT COUNT(`id`) AS c FROM `accounts` WHERE `name` = " << db->EscapeString(name);
    result = db->StoreQuery(query.str());
    if (!result)
        return ResultInternalError;
    if (result->GetUInt("c") != 0)
        return ResultNameExists;

    query.str("");
    query << "SELECT `id`, `account_key`, `used`, `total` FROM `account_keys` WHERE `status` = 1 AND `account_key` = " << db->EscapeString(accKey);
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
    query << static_cast<int>(AccountTypeNormal) << ", ";
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

bool IOAccount::LoginServerAuth(const std::string& name, const std::string& pass, Account& account)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `password`, `type` FROM `accounts` WHERE `name` = " << db->EscapeString(name);
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    std::string dbPass = result->GetString("password");
    if (bcrypt_checkpass(pass.c_str(), dbPass.c_str()) != 0)
        return false;

    account.id_ = result->GetUInt("id");
    account.name_ = result->GetString("name");
    account.type_ = static_cast<AccountType>(result->GetInt("type"));
    account.characters_.clear();

    std::string landingGame = IOGame::GetLandingGame();
    query.str("");
    query << "SELECT `id`, `level`, `name`, `profession`, `profession2`, `last_map` FROM `players` WHERE `deleted` = 0 AND `account_id` = " << account.id_;
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        std::string lastMap = result->GetString("last_map");
        if (lastMap.empty())
            lastMap = landingGame;
        AccountCharacter character
        {
            result->GetUInt("id"),
            static_cast<uint16_t>(result->GetUInt("level")),
            result->GetString("name"),
            result->GetString("profession"),
            result->GetString("profession2"),
            lastMap
        };
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

bool IOAccount::Save(const Account& account)
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
