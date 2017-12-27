#include "stdafx.h"
#include "SkillManager.h"
#include "DataProvider.h"

namespace Game {

SkillManager SkillManager::Instance;

SkillManager::SkillManager() :
    database_(nullptr)
{
}

bool SkillManager::Load(const std::string& file)
{
    database_ = std::make_unique<DB::DatabaseSqlite>(file);
    return (bool)database_;
}

std::shared_ptr<Skill> SkillManager::Get(uint32_t id)
{
    std::ostringstream query;
    query << "SELECT `id`, `script` FROM `skills` WHERE `id` = " << id;
    std::shared_ptr<DB::DBResult> dbRes = database_->StoreQuery(query.str());
    if (!dbRes)
        return std::shared_ptr<Skill>();

    std::shared_ptr<Skill> result = std::make_shared<Skill>(id);
    if (result->LoadScript(IO::DataProvider::Instance.GetDataFile(dbRes->GetString("script"))))
        return result;

    return std::shared_ptr<Skill>();
}

}
