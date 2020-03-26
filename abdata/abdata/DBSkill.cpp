/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "DBSkill.h"

namespace DB {

bool DBSkill::Create(AB::Entities::Skill& skill)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(skill.uuid))
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
    if (!Utils::Uuid::IsEmpty(skill.uuid))
        query << "`uuid` = " << db->EscapeString(skill.uuid);
    else
        // 0 is a valid index, it is the None skill
        query << "`idx` = " << skill.index;

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    skill.uuid = result->GetString("uuid");
    skill.index = result->GetUInt("idx");
    skill.name = result->GetString("name");
    skill.attributeUuid = result->GetString("attribute_uuid");
    skill.professionUuid = result->GetString("profession_uuid");
    skill.type = static_cast<AB::Entities::SkillType>(result->GetULong("type"));
    skill.isElite = result->GetUInt("is_elite") != 0;
    skill.description = result->GetString("description");
    skill.shortDescription = result->GetString("short_description");
    skill.icon = result->GetString("icon");
    skill.script = result->GetString("script");
    skill.access = result->GetUInt("access");
    skill.soundEffect = result->GetString("sound_effect");
    skill.particleEffect = result->GetString("particle_effect");
    skill.activation = result->GetInt("activation");
    skill.recharge = result->GetInt("recharge");
    skill.costEnergy = result->GetInt("const_energy");
    skill.costEnergyRegen = result->GetInt("const_energy_regen");
    skill.costAdrenaline = result->GetInt("const_adrenaline");
    skill.costOvercast = result->GetInt("const_overcast");
    skill.costHp = result->GetInt("const_hp");

    return true;
}

bool DBSkill::Save(const AB::Entities::Skill& skill)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(skill.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }
    return true;
}

bool DBSkill::Delete(const AB::Entities::Skill& skill)
{
    // Do nothing
    if (Utils::Uuid::IsEmpty(skill.uuid))
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
    if (!Utils::Uuid::IsEmpty(skill.uuid))
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
