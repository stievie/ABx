#include "stdafx.h"
#include "DBAttribute.h"
#include "Database.h"
#include "Logger.h"
#include "Subsystems.h"

namespace DB {

bool DBAttribute::Create(AB::Entities::Attribute&)
{
    // Do nothing
    return true;
}

bool DBAttribute::Load(AB::Entities::Attribute& attr)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `game_attributes` WHERE ";
    if (!Utils::Uuid::IsEmpty(attr.uuid))
        query << "`uuid` = " << db->EscapeString(attr.uuid);
    else if (attr.index != AB::Entities::INVALID_INDEX)
        query << "`idx` = " << attr.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    attr.uuid = result->GetString("uuid");
    attr.index = result->GetUInt("idx");
    attr.professionUuid = result->GetString("profession_uuid");
    attr.name = result->GetString("name");
    attr.isPrimary = result->GetUInt("is_primary") != 0;

    return true;
}

bool DBAttribute::Save(const AB::Entities::Attribute&)
{
    // Do nothing
    return true;
}

bool DBAttribute::Delete(const AB::Entities::Attribute&)
{
    // Do nothing
    return true;
}

bool DBAttribute::Exists(const AB::Entities::Attribute& attr)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) FROM `game_attributes` WHERE ";
    if (!Utils::Uuid::IsEmpty(attr.uuid))
        query << "`uuid` = " << db->EscapeString(attr.uuid);
    else if (attr.index != AB::Entities::INVALID_INDEX)
        query << "`idx` = " << attr.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
