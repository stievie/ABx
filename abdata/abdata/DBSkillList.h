#pragma once

#include <AB/Entities/SkillList.h>
namespace DB {

class DBSkillList
{
public:
    DBSkillList() = delete;
    ~DBSkillList() = delete;

    static bool Create(AB::Entities::SkillList&);
    static bool Load(AB::Entities::SkillList& sl);
    static bool Save(const AB::Entities::SkillList&);
    static bool Delete(const AB::Entities::SkillList&);
    static bool Exists(const AB::Entities::SkillList&);
};

}
