#include "stdafx.h"
#include "DBItemList.h"
#include "Database.h"

namespace DB {

bool DBItemList::Create(AB::Entities::ItemList&)
{
    return true;
}

bool DBItemList::Load(AB::Entities::ItemList& il)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `game_items`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        il.itemUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBItemList::Save(const AB::Entities::ItemList&)
{
    return true;
}

bool DBItemList::Delete(const AB::Entities::ItemList&)
{
    return true;
}

bool DBItemList::Exists(const AB::Entities::ItemList&)
{
    return true;
}

}
