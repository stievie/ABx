#include "stdafx.h"
#include "SkillManager.h"
#include "ConfigManager.h"

namespace Game {

SkillManager SkillManager::Instance;

std::shared_ptr<Skill> SkillManager::Get(uint32_t id)
{
    auto it = skills_.find(id);
    if (it == skills_.end())
        return std::shared_ptr<Skill>();

    std::shared_ptr<Skill> result = std::make_shared<Skill>(id);
    if (result->LoadScript(ConfigManager::Instance.GetDataFile((*it).second)))
        return result;

    return std::shared_ptr<Skill>();
}

}
