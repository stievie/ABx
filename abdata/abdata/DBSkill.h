#pragma once

#include <AB/Entities/Skill.h>

namespace DB {

class DBSkill
{
public:
    DBSkill() = delete;
    ~DBSkill() = delete;

    static bool Create(AB::Entities::Skill& skill);
    static bool Load(AB::Entities::Skill& skill);
    static bool Save(const AB::Entities::Skill& skill);
    static bool Delete(const AB::Entities::Skill& skill);
    static bool Exists(const AB::Entities::Skill& skill);
};

}
