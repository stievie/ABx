#include "stdafx.h"
#include "DBSkillList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBSkillList::Create(AB::Entities::SkillList&)
{
    return true;
}

bool DBSkillList::Load(AB::Entities::SkillList& sl)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `game_skills`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        sl.skillUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBSkillList::Save(const AB::Entities::SkillList&)
{
    return true;
}

bool DBSkillList::Delete(const AB::Entities::SkillList&)
{
    return true;
}

bool DBSkillList::Exists(const AB::Entities::SkillList&)
{
    return true;
}

}
