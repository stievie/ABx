#include "stdafx.h"
#include "DBInstance.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBInstance::Create(AB::Entities::GameInstance& inst)
{
    if (Utils::Uuid::IsEmpty(inst.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "INSERT INTO `instances` (`uuid`, `game_uuid`, `server_uuid`, `name`, `recording`, " <<
        "`start_time`, `stop_time`, `number`, `is_running`";
    query << ") VALUES (";

    query << db->EscapeString(inst.uuid) << ", ";
    query << db->EscapeString(inst.gameUuid) << ", ";
    query << db->EscapeString(inst.serverUuid) << ", ";
    query << db->EscapeString(inst.name) << ", ";
    query << db->EscapeString(inst.recording) << ", ";
    query << inst.startTime << ", ";
    query << inst.stopTime << ", ";
    query << inst.number << ", ";
    query << (inst.running ? 1 : 0);

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

bool DBInstance::Load(AB::Entities::GameInstance& inst)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `instances` WHERE ";
    if (!Utils::Uuid::IsEmpty(inst.uuid))
        query << "`uuid` = " << db->EscapeString(inst.uuid);
    else if (!inst.recording.empty())
        query << "`recording` = " << db->EscapeString(inst.recording);
    else
    {
        LOG_ERROR << "UUID and recording are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    inst.uuid = result->GetString("uuid");
    inst.gameUuid = result->GetString("game_uuid");
    inst.serverUuid = result->GetString("server_uuid");
    inst.name = result->GetString("name");
    inst.recording = result->GetString("recording");
    inst.startTime = result->GetLong("start_time");
    inst.stopTime = result->GetLong("stop_time");
    inst.number = static_cast<uint16_t>(result->GetUInt("number"));
    inst.running = result->GetUInt("is_running") != 0;

    return true;
}

bool DBInstance::Save(const AB::Entities::GameInstance& inst)
{
    if (Utils::Uuid::IsEmpty(inst.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `instances` SET ";
    query << " `game_uuid` = " << db->EscapeString(inst.gameUuid) << ", ";
    query << " `server_uuid` = " << db->EscapeString(inst.serverUuid) << ", ";
    query << " `name` = " << db->EscapeString(inst.name) << ", ";
    query << " `recording` = " << db->EscapeString(inst.recording) << ", ";
    query << " `start_time` = " << inst.startTime << ", ";
    query << " `stop_time` = " << inst.stopTime << ", ";
    query << " `number` = " << inst.number << ", ";
    query << " `is_running` = " << (inst.running ? 1 : 0);

    query << " WHERE `uuid` = " << db->EscapeString(inst.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBInstance::Delete(const AB::Entities::GameInstance& inst)
{
    if (Utils::Uuid::IsEmpty(inst.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `instances` WHERE `uuid` = " << db->EscapeString(inst.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBInstance::Exists(const AB::Entities::GameInstance& inst)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `instances` WHERE ";
    if (!Utils::Uuid::IsEmpty(inst.uuid))
        query << "`uuid` = " << db->EscapeString(inst.uuid);
    else if (!inst.recording.empty())
        query << "`recording` = " << db->EscapeString(inst.recording);
    else
    {
        LOG_ERROR << "UUID and recording are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
