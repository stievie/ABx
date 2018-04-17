#include "stdafx.h"
#include "DBAccount.h"
#include "Database.h"

namespace DB {

uint32_t DBAccount::Create(AB::Entities::Account& account)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    query << "INSERT INTO `accounts` (`name`, `password`, `email`, `type`, `blocked`, `creation`, `char_slots`) VALUES ( ";

    query << db->EscapeString(account.name) << ", ";
    query << db->EscapeString(account.password) << ", ";
    query << db->EscapeString(account.email) << ", ";
    query << static_cast<int>(account.type) << ", ";
    query << (account.blocked ? 1 : 0) << ", ";
    query << account.creation << ", ";
    query << account.charSlots;

    query << ")";
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return 0;

    if (!db->ExecuteQuery(query.str()))
        return 0;

    // End transaction
    if (!transaction.Commit())
        return 0;

    account.id = static_cast<uint32_t>(db->GetLastInsertId());
    return account.id;
}

bool DBAccount::Load(AB::Entities::Account& account)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `accounts` WHERE `id` = " << account.id;
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    account.name = result->GetString("name");
    account.password = result->GetString("password");
    account.email = result->GetString("email");
    account.type = static_cast<AB::Entities::AccountType>(result->GetInt("type"));
    account.blocked = result->GetUInt("blocked") != 0;
    account.creation = result->GetULong("creation");
    account.charSlots = result->GetUInt("char_slots");

    return true;
}

bool DBAccount::Save(const AB::Entities::Account& account)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    if (account.id == 0)
        return false;

    query << "UPDATE `accounts` SET ";

    query << " `password` = " << db->EscapeString(account.password) << ",";
    query << " `email` = " << db->EscapeString(account.email) << ",";
    query << " `type` = " << static_cast<int>(account.type) << ",";
    query << " `blocked` = " << (account.blocked ? 1 : 0) << ",";
    query << " `char_slots` = " << account.charSlots;

    query << " WHERE `id` = " << account.id;

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBAccount::Delete(const AB::Entities::Account& account)
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