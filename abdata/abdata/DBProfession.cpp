#include "stdafx.h"
#include "DBProfession.h"
#include "Database.h"
#include "Logger.h"

namespace DB {

bool DBProfession::Create(AB::Entities::Profession& prof)
{
    // Do nothing
    if (prof.uuid.empty() || uuids::uuid(prof.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBProfession::Load(AB::Entities::Profession& prof)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `game_professions` WHERE ";
    if (!prof.uuid.empty() && !uuids::uuid(prof.uuid).nil())
        query << "`uuid` = " << db->EscapeString(prof.uuid);
    else if (!prof.name.empty())
        query << "`name` = " << db->EscapeString(prof.name);
    else if (prof.index != 0)
        query << "`idx` = " << prof.index;
    else if (!prof.abbr.empty())
        query << "`abbr` = " << db->EscapeString(prof.abbr);
    else
    {
        LOG_ERROR << "UUID, name, abbr and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    prof.uuid = result->GetString("uuid");
    prof.index = result->GetUInt("idx");
    prof.name = result->GetString("name");
    prof.abbr = result->GetString("abbr");

    return true;
}

bool DBProfession::Save(const AB::Entities::Profession& prof)
{
    // Do nothing
    if (prof.uuid.empty() || uuids::uuid(prof.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBProfession::Delete(const AB::Entities::Profession& prof)
{
    // Do nothing
    if (prof.uuid.empty() || uuids::uuid(prof.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBProfession::Exists(const AB::Entities::Profession& prof)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_professions` WHERE ";
    if (!prof.uuid.empty() && !uuids::uuid(prof.uuid).nil())
        query << "`uuid` = " << db->EscapeString(prof.uuid);
    else if (prof.index != 0)
        query << "`idx` = " << prof.index;
    else if (!prof.name.empty())
        query << "`name` = " << db->EscapeString(prof.name);
    else if (!prof.abbr.empty())
        query << "`abbr` = " << db->EscapeString(prof.abbr);
    else
    {
        LOG_ERROR << "UUID, name, abbr and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
