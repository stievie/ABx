#include "stdafx.h"
#include "DBGuild.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBGuild::Create(AB::Entities::Guild& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `guilds` (`uuid`, `name`, `tag`, `creator_account_uuid`, `creation`";
    query << ") VALUES (";

    query << db->EscapeString(g.uuid) << ", ";
    query << db->EscapeString(g.name) << ", ";
    query << db->EscapeString(g.tag) << ", ";
    query << db->EscapeString(g.creatorAccountUuid) << ", ";
    query << g.creation;

    query << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBGuild::Load(AB::Entities::Guild& g)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `guilds` WHERE ";
    if (!Utils::Uuid::IsEmpty(g.uuid))
        query << "`uuid` = " << db->EscapeString(g.uuid);
    else if (!g.name.empty())
        query << "`name` = " << db->EscapeString(g.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    g.uuid = result->GetString("uuid");
    g.name = result->GetString("name");
    g.tag = result->GetString("tag");
    g.creatorAccountUuid = result->GetString("creator_account_uuid");
    g.creation = result->GetLong("creation");

    return true;
}

bool DBGuild::Save(const AB::Entities::Guild& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `guilds` SET ";

    // Only these may be changed
    query << " `name` = " << db->EscapeString(g.name) << ", ";
    query << " `tag` = " << db->EscapeString(g.tag) << ", ";
    query << " `creator_account_uuid` = " << db->EscapeString(g.creatorAccountUuid) << ", ";
    query << " `creation` = " << g.creation;

    query << " WHERE `uuid` = " << db->EscapeString(g.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBGuild::Delete(const AB::Entities::Guild& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `guilds` WHERE `uuid` = " << db->EscapeString(g.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBGuild::Exists(const AB::Entities::Guild& g)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `guilds` WHERE ";
    if (!Utils::Uuid::IsEmpty(g.uuid))
        query << "`uuid` = " << db->EscapeString(g.uuid);
    else if (!g.name.empty())
        query << "`name` = " << db->EscapeString(g.name);
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
