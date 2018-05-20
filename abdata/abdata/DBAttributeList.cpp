#include "stdafx.h"
#include "DBAttributeList.h"
#include "Database.h"

namespace DB {

bool DBAttributeList::Create(AB::Entities::AttributeList&)
{
    return true;
}

bool DBAttributeList::Load(AB::Entities::AttributeList& al)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `game_attributes`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        al.uuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBAttributeList::Save(const AB::Entities::AttributeList&)
{
    return true;
}

bool DBAttributeList::Delete(const AB::Entities::AttributeList&)
{
    return true;
}

bool DBAttributeList::Exists(const AB::Entities::AttributeList&)
{
    return true;
}

}
