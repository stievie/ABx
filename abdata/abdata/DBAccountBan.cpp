#include "stdafx.h"
#include "DBAccountBan.h"
#include "Database.h"
#include "Logger.h"

namespace DB {

bool DBAccountBan::Create(AB::Entities::AccountBan& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "INSERT INTO `account_bans` (`uuid`, `ban_uuid`, `account_uuid`) VALUES (";
    query << db->EscapeString(ban.uuid) << ", ";
    query << db->EscapeString(ban.banUuid) << ", ";
    query << db->EscapeString(ban.accountUuid);

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

bool DBAccountBan::Load(AB::Entities::AccountBan& ban)
{
    DB::Database* db = DB::Database::Instance();
    std::ostringstream query;
    query << "SELECT * FROM `account_bans` WHERE ";
    if (!ban.uuid.empty() && !uuids::uuid(ban.uuid).nil())
        query << "`uuid` = " << ban.uuid;
    else if (!ban.accountUuid.empty() && !uuids::uuid(ban.accountUuid).nil())
        query << "`account_uuid` = " << db->EscapeString(ban.accountUuid);
    else
    {
        LOG_ERROR << "UUID and Account UUID are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ban.uuid = result->GetString("uuid");
    ban.banUuid = result->GetString("ban_uuid");
    ban.accountUuid = result->GetString("account_uuid");

    return true;
}

bool DBAccountBan::Save(const AB::Entities::AccountBan& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;

    query << "UPDATE `account_bans` SET ";
    query << " `ban_uuid`" << ban.banUuid << ", ";
    query << " `account_uuid`" << ban.accountUuid;

    query << " WHERE `uuid` = " << db->EscapeString(ban.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBAccountBan::Delete(const AB::Entities::AccountBan& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `account_bans` WHERE `uuid` = " << db->EscapeString(ban.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBAccountBan::Exists(const AB::Entities::AccountBan& ban)
{
    DB::Database* db = DB::Database::Instance();
    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `account_bans` WHERE ";
    if (!ban.uuid.empty() && !uuids::uuid(ban.uuid).nil())
        query << "`uuid` = " << db->EscapeString(ban.uuid);
    else if (!ban.accountUuid.empty() && !uuids::uuid(ban.accountUuid).nil())
        query << "`account_uuid` = " << db->EscapeString(ban.accountUuid);
    else
    {
        LOG_ERROR << "UUID and Account UUID are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    return result->GetUInt("count") != 0;
}

}