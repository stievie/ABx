#include "stdafx.h"
#include "DBAccountKeyAccounts.h"
#include "Database.h"
#include "Logger.h"

namespace DB {

bool DBAccountKeyAccounts::Create(AB::Entities::AccountKeyAccounts& ak)
{
    if (ak.uuid.empty() || uuids::uuid(ak.uuid).nil() ||
        ak.accountUuid.empty() || uuids::uuid(ak.accountUuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "INSERT INTO `account_account_keys` (`account_uuid`, `account_key_uuid`) VALUES ( ";
    query << db->EscapeString(ak.accountUuid) << ",";
    query << db->EscapeString(ak.uuid);
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

bool DBAccountKeyAccounts::Load(AB::Entities::AccountKeyAccounts& ak)
{
    if (ak.uuid.empty() || uuids::uuid(ak.uuid).nil() ||
        ak.accountUuid.empty() || uuids::uuid(ak.accountUuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `account_account_keys` WHERE ";
    query << "`account_uuid` = " << db->EscapeString(ak.accountUuid);
    query << " AND `account_key_uuid` = " << db->EscapeString(ak.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ak.uuid = result->GetString("account_key_uuid");
    ak.accountUuid = result->GetString("account_uuid");
    return true;
}

bool DBAccountKeyAccounts::Save(const AB::Entities::AccountKeyAccounts&)
{
    // Not possible
    return false;
}

bool DBAccountKeyAccounts::Delete(const AB::Entities::AccountKeyAccounts&)
{
    // No possible
    return false;
}

bool DBAccountKeyAccounts::Exists(const AB::Entities::AccountKeyAccounts& ak)
{
    if (ak.uuid.empty() || uuids::uuid(ak.uuid).nil() ||
        ak.accountUuid.empty() || uuids::uuid(ak.accountUuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `account_account_keys` WHERE ";
    query << "`account_uuid` = " << db->EscapeString(ak.accountUuid);
    query << " AND `account_key_uuid` = " << db->EscapeString(ak.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    return result->GetUInt("count") != 0;
}

}
