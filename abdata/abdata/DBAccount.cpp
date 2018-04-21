#include "stdafx.h"
#include "DBAccount.h"
#include "Database.h"
#include "Logger.h"
#include <uuid.h>

namespace DB {

bool DBAccount::Create(AB::Entities::Account& account)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    query << "INSERT INTO `accounts` (`uuid`, `name`, `password`, `email`, `type`, `blocked`, `creation`, `char_slots`) VALUES ( ";

    if (account.uuid.empty())
    {
        LOG_WARNING << "UUID of entity is empty" << std::endl;
        const uuids::uuid guid = uuids::uuid_random_generator{}();
        account.uuid = guid.to_string();
    }
    query << db->EscapeString(account.uuid) << ", ";
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
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBAccount::Load(AB::Entities::Account& account)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `accounts` WHERE ";
    if (!account.uuid.empty() && !uuids::uuid(account.uuid).nil())
        query << "`uuid` = " << db->EscapeString(account.uuid);
    else if (!account.name.empty())
        query << "`name` = " << db->EscapeString(account.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    account.uuid = result->GetString("uuid");
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
    if (account.uuid.empty() || uuids::uuid(account.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;

    query << "UPDATE `accounts` SET ";

    query << " `password` = " << db->EscapeString(account.password) << ",";
    query << " `email` = " << db->EscapeString(account.email) << ",";
    query << " `type` = " << static_cast<int>(account.type) << ",";
    query << " `blocked` = " << (account.blocked ? 1 : 0) << ",";
    query << " `char_slots` = " << account.charSlots;

    query << " WHERE `uuid` = " << db->EscapeString(account.uuid);

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
    if (account.uuid.empty() || uuids::uuid(account.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `accounts` WHERE `uuid` = " << db->EscapeString(account.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

}