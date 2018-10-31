#include "stdafx.h"
#include "DBEffectList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBEffectList::Create(AB::Entities::EffectList&)
{
    return true;
}

bool DBEffectList::Load(AB::Entities::EffectList& el)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `game_effects`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        el.effectUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBEffectList::Save(const AB::Entities::EffectList&)
{
    return true;
}

bool DBEffectList::Delete(const AB::Entities::EffectList&)
{
    return true;
}

bool DBEffectList::Exists(const AB::Entities::EffectList&)
{
    return true;
}

}
