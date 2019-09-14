#include "stdafx.h"
#include "DBVersionList.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBVersionList::Create(AB::Entities::VersionList&)
{
    return true;
}

bool DBVersionList::Load(AB::Entities::VersionList& vl)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `versions` WHERE `internal` = 0";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        vl.versions.push_back({
            result->GetString("uuid"),
            result->GetString("name"),
            result->GetUInt("value"),
            false
        });
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
