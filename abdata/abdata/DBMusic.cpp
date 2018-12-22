#include "stdafx.h"
#include "DBMusic.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBMusic::Create(AB::Entities::Music& item)
{
    // Do nothing
    if (item.uuid.empty() || uuids::uuid(item.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "INSERT INTO `game_music` (`uuid`, `map_uuid`, `local_file`, `remote_file`, `sorting`, " <<
        "`style`";
    query << ") VALUES (";

    query << db->EscapeString(item.uuid) << ", ";
    query << db->EscapeString(item.mapUuid) << ", ";
    query << db->EscapeString(item.localFile) << ", ";
    query << db->EscapeString(item.remoteFile) << ", ";
    query << static_cast<int>(item.sorting) << ", ";
    query << static_cast<int>(item.style);

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

bool DBMusic::Load(AB::Entities::Music& item)
{
    if (item.uuid.empty() || uuids::uuid(item.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `game_music` WHERE ";
    query << "`uuid` = " << db->EscapeString(item.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    item.uuid = result->GetString("uuid");
    item.mapUuid = result->GetString("map_uuid");
    item.localFile = result->GetString("local_file");
    item.remoteFile = result->GetString("remote_file");
    item.sorting = static_cast<uint8_t>(result->GetUInt("sorting"));
    item.style = static_cast<AB::Entities::MusicStyle>(result->GetUInt("style"));

    return true;
}

bool DBMusic::Save(const AB::Entities::Music& item)
{
    if (item.uuid.empty() || uuids::uuid(item.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `game_music` SET ";
    query << " `map_uuid` = " << db->EscapeString(item.mapUuid) << ", ";
    query << " `local_file` = " << db->EscapeString(item.localFile) << ", ";
    query << " `remote_file` = " << db->EscapeString(item.remoteFile) << ", ";
    query << " `sorting` = " << static_cast<int>(item.sorting) << ", ";
    query << " `style` = " << static_cast<int>(item.style);

    query << " WHERE `uuid` = " << db->EscapeString(item.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBMusic::Delete(const AB::Entities::Music& item)
{
    if (item.uuid.empty() || uuids::uuid(item.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `game_music` WHERE `uuid` = " << db->EscapeString(item.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBMusic::Exists(const AB::Entities::Music& item)
{
    if (item.uuid.empty() || uuids::uuid(item.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_effects` WHERE ";
    query << "`uuid` = " << db->EscapeString(item.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
