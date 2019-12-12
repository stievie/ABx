#include "stdafx.h"
#include "DBPlayerQuestList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBPlayerQuestList::Create(AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestList::Load(AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `quest_uuid` FROM `player_quests` WHERE ";
    query << "`player_uuid` = " << db->EscapeString(g.uuid);

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        g.questUuids.push_back(
            result->GetString("quest_uuid")
        );
    }

    return true;
}

bool DBPlayerQuestList::Save(const AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestList::Delete(const AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestList::Exists(const AB::Entities::PlayerQuestList& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `player_quests` WHERE ";
    query << "`player_uuid` = " << db->EscapeString(g.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
