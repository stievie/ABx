#include "stdafx.h"
#include "DBService.h"
#include "Database.h"

namespace DB {

bool DBService::Create(AB::Entities::Service& s)
{
    if (s.uuid.empty() || uuids::uuid(s.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;

    query << "INSERT INTO `services` (`uuid`, `name`, `type`, `host`, `port`, `status`, `start_time`, `stop_time`, `run_time`) VALUES (";

    query << db->EscapeString(s.uuid) << ", ";
    query << db->EscapeString(s.name) << ", ";
    query << static_cast<int>(s.type) << ", ";
    query << db->EscapeString(s.host) << ", ";
    query << s.port << ", ";
    query << static_cast<int>(s.status) << ", ";
    query << s.startTime << ", ";
    query << s.stopTime << ", ";
    query << s.runTime;

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

bool DBService::Load(AB::Entities::Service& s)
{
    if (s.uuid.empty() || uuids::uuid(s.uuid).nil())
    {
        return false;
        LOG_ERROR << "UUID is empty" << std::endl;
    }

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `services` WHERE ";
    query << "`uuid` = " << db->EscapeString(s.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    s.uuid = result->GetString("uuid");
    s.name = result->GetString("name");
    s.type = static_cast<AB::Entities::ServiceType>(result->GetUInt("type"));
    s.host = result->GetString("host");
    s.port = static_cast<uint16_t>(result->GetUInt("port"));
    s.status = static_cast<AB::Entities::ServiceStatus>(result->GetUInt("status"));
    s.startTime = result->GetLong("start_time");
    s.stopTime = result->GetLong("stop_time");
    s.runTime = result->GetLong("run_time");

    return true;
}

bool DBService::Save(const AB::Entities::Service& s)
{
    if (s.uuid.empty() || uuids::uuid(s.uuid).nil())
    {
        return false;
        LOG_ERROR << "UUID is empty" << std::endl;
    }

    Database* db = Database::Instance();
    std::ostringstream query;

    query << "UPDATE `services` SET ";
    query << " `name` = " << db->EscapeString(s.name) << ", ";
    query << " `type` = " << static_cast<int>(s.type) << ", ";
    query << " `host` = " << db->EscapeString(s.host) << ", ";
    query << " `port` = " << s.port << ", ";
    query << " `status` = " << static_cast<int>(s.status) << ", ";
    query << " `start_time` = " << s.startTime << ", ";
    query << " `stop_time` = " << s.stopTime << ", ";
    query << " `run_time` = " << s.runTime;

    query << " WHERE `uuid` = " << db->EscapeString(s.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBService::Delete(const AB::Entities::Service& s)
{
    if (s.uuid.empty() || uuids::uuid(s.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `services` WHERE `uuid` = " << db->EscapeString(s.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBService::Exists(const AB::Entities::Service& s)
{
    if (s.uuid.empty() || uuids::uuid(s.uuid).nil())
    {
        return false;
        LOG_ERROR << "UUID is empty" << std::endl;
    }

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `services` WHERE ";
    query << "`uuid` = " << db->EscapeString(s.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
