#include "stdafx.h"
#include "DBProfession.h"
#include "Database.h"
#include "Subsystems.h"

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
    Database* db = GetSubsystem<Database>();

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
    prof.modelIndexFemale = result->GetUInt("model_index_female");
    prof.modelIndexMale = result->GetUInt("model_index_male");

    // Get attributes
    query.str("");
    query << "SELECT `uuid`, `idx` FROM `game_attributes` WHERE `profession_uuid` = " << db->EscapeString(prof.uuid);
    query << " ORDER BY `idx`";
    std::shared_ptr<DB::DBResult> resAttrib = db->StoreQuery(query.str());
    prof.attributeCount= 0;
    prof.attributeUuids.clear();
    prof.attributeIndices.clear();
    if (resAttrib)
    {
        for (resAttrib = db->StoreQuery(query.str()); resAttrib; resAttrib = resAttrib->Next())
        {
            ++prof.attributeCount;
            prof.attributeUuids.push_back(resAttrib->GetString("uuid"));
            prof.attributeIndices.push_back(resAttrib->GetUInt("idx"));
        }
    }
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
    Database* db = GetSubsystem<Database>();

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
