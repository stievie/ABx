#include "stdafx.h"
#include "DBReservedName.h"
#include "Database.h"

namespace DB {

bool DBReservedName::Create(AB::Entities::ReservedName&)
{
    // Do nothing
    return true;
}

bool DBReservedName::Load(AB::Entities::ReservedName& n)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `reserved_names` WHERE ";
    if (!n.uuid.empty() && !uuids::uuid(n.uuid).nil())
        query << "`uuid` = " << db->EscapeString(n.uuid);
    else if (!n.name.empty())
        query << "`name` = " << db->EscapeString(n.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    n.uuid = result->GetString("uuid");
    n.name = result->GetString("name");
    n.isReserved = result->GetInt("is_reserved") != 0;

    return true;
}

bool DBReservedName::Save(const AB::Entities::ReservedName&)
{
    // Do nothing
    return true;
}

bool DBReservedName::Delete(const AB::Entities::ReservedName&)
{
    // Do nothing
    return true;
}

bool DBReservedName::Exists(const AB::Entities::ReservedName& n)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `reserved_names` WHERE ";
    if (!n.uuid.empty() && !uuids::uuid(n.uuid).nil())
        query << "`uuid` = " << db->EscapeString(n.uuid);
    else if (!n.name.empty())
        query << "`name` = " << db->EscapeString(n.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }
    if (n.isReserved)
        query << " AND `is_reserved` = 1";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
