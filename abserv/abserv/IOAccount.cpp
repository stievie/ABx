#include "stdafx.h"
#include "IOAccount.h"
#include "Database.h"
#include <abcrypto.hpp>

namespace DB {

bool IOAccount::LoginServerAuth(const std::string & name, const std::string & pass, Account& account)
{
    Database* db = Database::Instance();

    std::ostringstream query;
    query << "SELECT `id`, `name`, `password`, `type` FROM `accounts` WHERE `name` = " << db->EscapeString(name);
    std::shared_ptr<DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    if (bcrypt_checkpass(pass.c_str(), result->GetString("password").c_str()) != 0)
        return false;

    account.id_ = result->GetUInt("id");
    account.name_ = result->GetString("name");
    account.type_ = static_cast<AccountType>(result->GetInt("type"));
    account.characters_.clear();

    query.str("");
    query << "SELECT `id`, `level`, `name` FROM `players` WHERE `deleted` = 0 AND `account_id` = " << account.id_;
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        AccountCharacter character
        {
            result->GetUInt("id"),
            static_cast<uint16_t>(result->GetUInt("level")),
            result->GetString("name")
        };
        account.characters_.push_back(character);
    }
    account.loggedIn_ = true;
    return true;
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
