#include "stdafx.h"
#include "DBProfessionList.h"
#include "Database.h"

namespace DB {

bool DBProfessionList::Create(AB::Entities::ProfessionList&)
{
    return true;
}

bool DBProfessionList::Load(AB::Entities::ProfessionList& pl)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `game_professions`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        pl.profUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBProfessionList::Save(const AB::Entities::ProfessionList&)
{
    return true;
}

bool DBProfessionList::Delete(const AB::Entities::ProfessionList&)
{
    return true;
}

bool DBProfessionList::Exists(const AB::Entities::ProfessionList&)
{
    return true;
}

}
