#include "stdafx.h"
#include "DBEffect.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBEffect::Create(AB::Entities::Effect& effect)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(effect.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Load(AB::Entities::Effect& effect)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `game_effects` WHERE ";
    if (!Utils::Uuid::IsEmpty(effect.uuid))
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
    effect.soundEffect = result->GetString("sound_effect");
    effect.particleEffect = result->GetString("particle_effect");

    return true;
}

bool DBEffect::Save(const AB::Entities::Effect& effect)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(effect.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Delete(const AB::Entities::Effect& effect)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(effect.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBEffect::Exists(const AB::Entities::Effect& effect)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_effects` WHERE ";
    if (!Utils::Uuid::IsEmpty(effect.uuid))
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
