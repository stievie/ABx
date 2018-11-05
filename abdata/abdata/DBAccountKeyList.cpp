#include "stdafx.h"
#include "DBAccountKeyList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBAccountKeyList::Create(AB::Entities::AccountKeyList&)
{
    return true;
}

bool DBAccountKeyList::Load(AB::Entities::AccountKeyList& al)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `account_keys`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        al.uuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBAccountKeyList::Save(const AB::Entities::AccountKeyList&)
{
    return true;
}

bool DBAccountKeyList::Delete(const AB::Entities::AccountKeyList&)
{
    return true;
}

bool DBAccountKeyList::Exists(const AB::Entities::AccountKeyList&)
{
    return true;
}

}
