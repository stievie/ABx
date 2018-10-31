#include "stdafx.h"
#include "DBSkill.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBSkill::Create(AB::Entities::Skill& skill)
{
    // Do nothing
    if (skill.uuid.empty() || uuids::uuid(skill.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBSkill::Load(AB::Entities::Skill& skill)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `game_skills` WHERE ";
    if (!skill.uuid.empty() && !uuids::uuid(skill.uuid).nil())
        query << "`uuid` = " << db->EscapeString(skill.uuid);
    else if (skill.index != 0)
        query << "`idx` = " << skill.index;
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    skill.uuid = result->GetString("uuid");
    skill.index = result->GetUInt("idx");
    skill.name = result->GetString("name");
    skill.attributeUuid = result->GetString("attribute_uuid");
    skill.type = static_cast<AB::Entities::SkillType>(result->GetUInt("type"));
    skill.isElite = result->GetUInt("is_elite") != 0;
    skill.description = result->GetString("description");
    skill.shortDescription = result->GetString("short_description");
    skill.icon = result->GetString("icon");
    skill.script = result->GetString("script");
    skill.isLocked = result->GetUInt("is_locked") != 0;

    return true;
}

bool DBSkill::Save(const AB::Entities::Skill& skill)
{
    // Do nothing
    if (skill.uuid.empty() || uuids::uuid(skill.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBSkill::Delete(const AB::Entities::Skill& skill)
{
    // Do nothing
    if (skill.uuid.empty() || uuids::uuid(skill.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBSkill::Exists(const AB::Entities::Skill& skill)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `game_skills` WHERE ";
    if (!skill.uuid.empty() && !uuids::uuid(skill.uuid).nil())
        query << "`uuid` = " << db->EscapeString(skill.uuid);
    else if (skill.index != 0)
        query << "`idx` = " << skill.index;
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
