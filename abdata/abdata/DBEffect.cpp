#include "stdafx.h"
#include "DBEffect.h"
#include "Database.h"

namespace DB {

bool DBEffect::Create(AB::Entities::Effect& effect)
{
    // Do nothing
    if (effect.uuid.empty() || uuids::uuid(effect.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Load(AB::Entities::Effect& effect)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `game_effects` WHERE ";
    if (!effect.uuid.empty() && !uuids::uuid(effect.uuid).nil())
        query << "`uuid` = " << db->EscapeString(effect.uuid);
    else if (effect.index != 0)
        query << "`idx` = " << effect.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    effect.uuid = result->GetString("uuid");
    effect.index = result->GetUInt("idx");
    effect.name = result->GetString("name");
    effect.category = static_cast<AB::Entities::EffectCategory>(result->GetUInt("category"));
    effect.script = result->GetString("script");
    effect.icon = result->GetString("icon");

    return true;
}

bool DBEffect::Save(const AB::Entities::Effect& effect)
{
    // Do nothing
    if (effect.uuid.empty() || uuids::uuid(effect.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Delete(const AB::Entities::Effect& effect)
{
    // Do nothing
    if (effect.uuid.empty() || uuids::uuid(effect.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Exists(const AB::Entities::Effect& effect)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_effects` WHERE ";
    if (!effect.uuid.empty() && !uuids::uuid(effect.uuid).nil())
        query << "`uuid` = " << db->EscapeString(effect.uuid);
    else if (effect.index != 0)
        query << "`idx` = " << effect.index;
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
