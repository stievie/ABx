#include "stdafx.h"
#include "DBVersionList.h"
#include "Database.h"

namespace DB {

bool DBVersionList::Create(AB::Entities::VersionList&)
{
    return true;
}

bool DBVersionList::Load(AB::Entities::VersionList& vl)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `versions`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        vl.versionUuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBVersionList::Save(const AB::Entities::VersionList&)
{
    return true;
}

bool DBVersionList::Delete(const AB::Entities::VersionList&)
{
    return true;
}

bool DBVersionList::Exists(const AB::Entities::VersionList&)
{
    return true;
}

}
