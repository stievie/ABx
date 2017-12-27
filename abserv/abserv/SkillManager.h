#pragma once

#include "Skill.h"
#include "DatabaseSqlite.h"

namespace Game {

class SkillManager
{
private:
    std::unique_ptr<DB::DatabaseSqlite> database_;
public:
    SkillManager();
    ~SkillManager() = default;

    bool Load(const std::string& file);

    std::shared_ptr<Skill> Get(uint32_t id);
public:
    static SkillManager Instance;
};

}
