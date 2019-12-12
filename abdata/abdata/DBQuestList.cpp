#include "stdafx.h"
#include "DBQuestList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBQuestList::Create(AB::Entities::QuestList&)
{
    return true;
}

bool DBQuestList::Load(AB::Entities::QuestList& q)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `game_quests`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        q.questUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBQuestList::Save(const AB::Entities::QuestList&)
{
    return true;
}

bool DBQuestList::Delete(const AB::Entities::QuestList&)
{
    return true;
}

bool DBQuestList::Exists(const AB::Entities::QuestList&)
{
    return true;
}

}
