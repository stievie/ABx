#include "stdafx.h"
#include "DBAccount.h"
#include "Database.h"

namespace DB {

bool DBAccount::Load(Entities::Account& account)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM accounts WHERE `id` = " << account.id;
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    account.name = result->GetString("name");
    account.password = result->GetString("password");
    account.email = result->GetString("email");
    account.type = static_cast<Entities::AccountType>(result->GetInt("type"));
    account.blocked = result->GetUInt("blocked") != 0;
    account.creation = result->GetULong("creation");
    account.charSlots = result->GetUInt("char_slots");

    return true;
}

bool DBAccount::Save(Entities::Account& account)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    if (account.id != 0)
    {
        query << "UPDATE `accounts` SET ";

        query << " `password` = " << db->EscapeString(account.password) << ",";
        query << " `email` = " << db->EscapeString(account.email) << ",";
        query << " `type` = " << static_cast<int>(account.type) << ",";
        query << " `blocked` = " << (account.blocked ? 1 : 0) << ",";
        query << " `char_slots` = " << account.charSlots << ",";

        query << " WHERE `id` = " << account.id;
    }
    else
    {
        query << "INSERT INTO `accounts` (`name`, `password`, `email`, `type`, `blocked`, `creation`, `char_slots`) VALUES ( ";

        query << db->EscapeString(account.name) << ", ";
        query << db->EscapeString(account.password) << ", ";
        query << db->EscapeString(account.email) << ", ";
        query << static_cast<int>(account.type) << ", ";
        query << (account.blocked ? 1 : 0) << ", ";
        query << account.creation << ", ";
        query << account.charSlots;

        query << ")";
    }

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    bool ret = transaction.Commit();
    if (ret && account.id == 0)
        account.id = static_cast<uint32_t>(db->GetLastInsertId());
    return ret;
}

bool DBAccount::Delete(Entities::Account& account)
{
    if (account.id == 0)
        return false;

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `accounts` WHERE `id` = " << account.id;
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

}