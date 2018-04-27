#include "stdafx.h"
#include "DBIpBan.h"
#include "Database.h"
#include "Logger.h"

namespace DB {

bool DBIpBan::Create(AB::Entities::IpBan& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    DB::DBQuery dbQuery;
    dbQuery << "SELECT COUNT(*) as `count` FROM `ip_bans` WHERE ";
    dbQuery << "((" << ban.ip << " & " << ban.mask << " & `mask`) = (`ip` & `mask` & " << ban.mask << "))";
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(dbQuery);
    if (result && result->GetInt("count") != 0)
    {
        LOG_ERROR << "There is already a record matching this IP and mask" << std::endl;
        return false;
    }

    std::ostringstream query;
    query << "INSERT INTO `ip_bans` (`uuid`, `ban_uuid`, `ip`, `mask`) VALUES (";
    query << db->EscapeString(ban.uuid) << ", ";
    query << db->EscapeString(ban.banUuid) << ", ";
    query << ban.ip << ", ";
    query << ban.mask;

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

bool DBIpBan::Load(AB::Entities::IpBan& ban)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `ip_bans` WHERE ";
    if (!ban.uuid.empty() && !uuids::uuid(ban.uuid).nil())
        query << "`uuid` = " << ban.uuid;
    else if (!ban.ip != 0)
    {
        if (ban.mask == 0)
        {
            LOG_ERROR << "IP mask is 0 it would match all IPs" << std::endl;
            return false;
        }
        query << "((" << ban.ip << " & " << ban.mask << " & `mask`) = (`ip` & `mask` & " << ban.mask << "))";
    }
    else
    {
        LOG_ERROR << "UUID and IP are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ban.uuid = result->GetString("uuid");
    ban.banUuid = result->GetString("ban_uuid");
    ban.ip = result->GetUInt("ip");
    ban.mask = result->GetUInt("mask");

    return true;
}

bool DBIpBan::Save(const AB::Entities::IpBan& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;

    query << "UPDATE `ip_bans` SET ";
    query << " `ban_uuid`" << db->EscapeString(ban.banUuid) << ", ";
    query << " `ip`" << ban.ip << ", ";
    query << " `mask`" << ban.ip;

    query << " WHERE `uuid` = " << db->EscapeString(ban.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBIpBan::Delete(const AB::Entities::IpBan& ban)
{
    if (ban.uuid.empty() || uuids::uuid(ban.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `ip_bans` WHERE `uuid` = " << db->EscapeString(ban.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBIpBan::Exists(const AB::Entities::IpBan& ban)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `ip_bans` WHERE ";
    if (!ban.uuid.empty() && !uuids::uuid(ban.uuid).nil())
        query << "`uuid` = " << ban.uuid;
    else if (!ban.ip != 0)
    {
        if (ban.mask == 0)
        {
            LOG_ERROR << "IP mask is 0 it would match all IPs" << std::endl;
            return false;
        }
        query << "((" << ban.ip << " & " << ban.mask << " & `mask`) = (`ip` & `mask` & " << ban.mask << "))";
    }
    else
    {
        LOG_ERROR << "UUID and IP are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}