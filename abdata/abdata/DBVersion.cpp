#include "stdafx.h"
#include "DBVersion.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBVersion::Create(AB::Entities::Version&)
{
    // Do nothing
    return true;
}

bool DBVersion::Load(AB::Entities::Version& v)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `versions` WHERE ";
    if (!v.uuid.empty() && !uuids::uuid(v.uuid).nil())
        query << "`uuid` = " << db->EscapeString(v.uuid);
    else if (!v.name.empty())
        query << "`name` = " << db->EscapeString(v.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    v.uuid = result->GetString("uuid");
    v.name = result->GetString("name");
    v.value = result->GetUInt("value");
    v.isInternal = result->GetUInt("internal") != 0;

    return true;
}

bool DBVersion::Save(const AB::Entities::Version&)
{
    // Do nothing
    return true;
}

bool DBVersion::Delete(const AB::Entities::Version&)
{
    // Do nothing
    return true;
}

bool DBVersion::Exists(const AB::Entities::Version& v)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `versions` WHERE ";
    if (!v.uuid.empty() && !uuids::uuid(v.uuid).nil())
        query << "`uuid` = " << db->EscapeString(v.uuid);
    else if (!v.name.empty())
        query << "`name` = " << db->EscapeString(v.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
