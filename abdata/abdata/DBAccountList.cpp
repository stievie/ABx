#include "stdafx.h"
#include "DBAccountList.h"
#include "Database.h"

namespace DB {

bool DBAccountList::Create(AB::Entities::AccountList&)
{
    return true;
}

bool DBAccountList::Load(AB::Entities::AccountList& al)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `accounts`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        al.uuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBAccountList::Save(const AB::Entities::AccountList&)
{
    return true;
}

bool DBAccountList::Delete(const AB::Entities::AccountList&)
{
    return true;
}

bool DBAccountList::Exists(const AB::Entities::AccountList&)
{
    return true;
}

}
