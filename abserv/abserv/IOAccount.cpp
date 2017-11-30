#include "stdafx.h"
#include "IOAccount.h"
#include "Database.h"
#include <abcrypto.hpp>
#include "IOGame.h"

#include "DebugNew.h"

namespace DB {

bool IOAccount::LoginServerAuth(const std::string & name, const std::string & pass, Account& account)
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
    query << "SELECT `id`, `level`, `name`, `last_map` FROM `players` WHERE `deleted` = 0 AND `account_id` = " << account.id_;
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

bool IOAccount::Load(Account& account, uint32_t id)
{
    return false;
}

}
