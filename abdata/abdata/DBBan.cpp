#include "stdafx.h"
#include "DBBan.h"
#include "Database.h"

namespace DB {

bool DBBan::Create(AB::Entities::Ban& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "INSERT INTO `bans` (`uuid`, `expires`, `added`, `reason`, `active`, `admin_uuid`, `comment`) VALUES (";
    query << db->EscapeString(ban.uuid) << ", ";
    query << ban.expires << ", ";
    query << ban.added << ", ";
    query << static_cast<int>(ban.reason) << ", ";
    query << static_cast<int>(ban.active) << ", ";
    query << db->EscapeString(ban.adminUuid) << ", ";
    query << db->EscapeString(ban.comment);

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

bool DBBan::Load(AB::Entities::Ban& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `bans` WHERE `uuid` = " << db->EscapeString(ban.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ban.uuid = result->GetString("uuid");
    ban.expires = result->GetLong("expires");
    ban.added = result->GetLong("expires");
    ban.reason = static_cast<AB::Entities::BanReason>(result->GetInt("reason"));
    ban.active = result->GetUInt("active") != 0;
    ban.adminUuid = result->GetString("admin_uuid");
    ban.comment = result->GetString("comment");

    return true;
}

bool DBBan::Save(const AB::Entities::Ban& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;

    query << "UPDATE `bans` SET ";
    query << " `expires`" << ban.expires << ", ";
    query << " `added`" << ban.added << ", ";
    query << " `reason`" << static_cast<int>(ban.reason) << ", ";
    query << " `active`" << (ban.active ? 1 : 0) << ", ";
    query << " `admin_uuid`" << db->EscapeString(ban.adminUuid) << ", ";
    query << " `comment`" << db->EscapeString(ban.comment);

    query << " WHERE `uuid` = " << db->EscapeString(ban.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBBan::Delete(const AB::Entities::Ban& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `bans` WHERE `uuid` = " << db->EscapeString(ban.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBBan::Exists(const AB::Entities::Ban& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `bans` WHERE `uuid` = " << db->EscapeString(ban.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}