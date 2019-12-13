#include "stdafx.h"
#include "DBPlayerQuestListRewarded.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBPlayerQuestListRewarded::Create(AB::Entities::PlayerQuestListRewarded& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestListRewarded::Load(AB::Entities::PlayerQuestListRewarded& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `quest_uuid` FROM `player_quests` WHERE ";
    query << "`player_uuid` = " << db->EscapeString(g.uuid) << " AND ";
    query << "`rewarded` = 1";

    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        g.questUuids.push_back(
            result->GetString("quest_uuid")
        );
    }

    return true;
}

bool DBPlayerQuestListRewarded::Save(const AB::Entities::PlayerQuestListRewarded& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestListRewarded::Delete(const AB::Entities::PlayerQuestListRewarded& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBPlayerQuestListRewarded::Exists(const AB::Entities::PlayerQuestListRewarded& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `player_quests` WHERE ";
    query << "`player_uuid` = " << db->EscapeString(g.uuid) << " AND ";
    query << "`rewarded` = 1";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
