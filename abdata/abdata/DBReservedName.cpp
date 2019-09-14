#include "stdafx.h"
#include "DBReservedName.h"
#include "Database.h"
#include "StorageProvider.h"
#include "Subsystems.h"

namespace DB {

// Player names are case insensitive. The DB needs a proper index for that:
// CREATE INDEX reserved_names_name_ci_index ON reserved_names USING btree (lower(name))

bool DBReservedName::Create(AB::Entities::ReservedName&)
{
    // Do nothing
    return true;
}

bool DBReservedName::Load(AB::Entities::ReservedName& n)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `reserved_names` WHERE ";
    if (!n.uuid.empty() && !uuids::uuid(n.uuid).nil())
        query << "`uuid` = " << db->EscapeString(n.uuid);
    else if (!n.name.empty())
        query << "LOWER(`name`) = LOWER(" << db->EscapeString(n.name) << ")";
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
    n.reservedForAccountUuid = result->GetString("reserved_for_account_uuid");
    n.expires = result->GetLong("expires");

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
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `reserved_names` WHERE ";
    if (!n.uuid.empty() && !uuids::uuid(n.uuid).nil())
        query << "`uuid` = " << db->EscapeString(n.uuid);
    else if (!n.name.empty())
        query << "LOWER(`name`) = LOWER(" << db->EscapeString(n.name) << ")";
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

void DBReservedName::DeleteExpired(StorageProvider* sp)
{
    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "SELECT `uuid` FROM `reserved_names` WHERE ";
    query << "(`expires` <> 0 AND `expires` < " << Utils::Tick() << ")";

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        // No matches
        return;

    std::vector<std::string> rns;
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        rns.push_back(result->GetString("uuid"));
    }

    // First invalidate them
    for (const std::string& g : rns)
    {
        AB::Entities::ReservedName rn;
        rn.uuid = g;
        sp->EntityInvalidate(rn);
    }

    // Then delete from DB
    query.str("");
    query << "DELETE FROM `reserved_names` WHERE ";
    query << "(`expires` <> 0 AND `expires` < " << Utils::Tick() << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return;

    if (!db->ExecuteQuery(query.str()))
        return;

    // End transaction
    if (!transaction.Commit())
        return;
}

}
