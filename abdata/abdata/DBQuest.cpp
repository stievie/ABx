#include "stdafx.h"
#include "DBQuest.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBQuest::Create(AB::Entities::Quest& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `game_quests` (`uuid`, `idx`, `name`, `script`, `repeatable`, `description` ";
    query << ") VALUES (";

    query << db->EscapeString(v.uuid) << ", ";
    query << v.index << ", ";
    query << db->EscapeString(v.name) << ", ";
    query << db->EscapeString(v.script) << ", ";
    query << (v.repeatable ? 1 : 0) << ", ";
    query << db->EscapeString(v.description);

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

bool DBQuest::Load(AB::Entities::Quest& v)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `game_quests` WHERE ";
    if (!Utils::Uuid::IsEmpty(v.uuid))
        query << "`uuid` = " << db->EscapeString(v.uuid);
    else if (v.index != AB::Entities::INVALID_INDEX)
        query << "idx = " << v.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    v.uuid = result->GetString("uuid");
    v.index = result->GetUInt("idx");
    v.name = result->GetString("name");
    v.script = result->GetString("script");
    v.repeatable = result->GetUInt("repeatable") != 0;
    v.description = result->GetString("description");

    return true;
}

bool DBQuest::Save(const AB::Entities::Quest& v)
{
    if (Utils::Uuid::IsEmpty(v.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `game_quests` SET ";

    // Only these may be changed
    query << " `name` = " << db->EscapeString(v.name) << ", ";
    query << " `script` = " << db->EscapeString(v.script) << ", ";
    query << " `repeatable` = " << (v.repeatable ? 1 : 0) << ", ";
    query << " `description` = " << db->EscapeString(v.description);

    query << " WHERE `uuid` = " << db->EscapeString(v.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBQuest::Delete(const AB::Entities::Quest&)
{
    // Do nothing
    return true;
}

bool DBQuest::Exists(const AB::Entities::Quest& v)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_quests` WHERE ";
    if (!Utils::Uuid::IsEmpty(v.uuid))
        query << "`uuid` = " << db->EscapeString(v.uuid);
    else if (v.index != AB::Entities::INVALID_INDEX)
        query << "idx = " << v.index << ")";
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
