#include "stdafx.h"
#include "SkillManager.h"
#include "DataProvider.h"
#include <AB/Entities/Skill.h>

namespace Game {

SkillManager SkillManager::Instance;

SkillManager::SkillManager()
{
}

std::shared_ptr<Skill> SkillManager::Get(uint32_t index)
{
    std::shared_ptr<Skill> result;
    auto it = skillCache_.find(index);
    if (it != skillCache_.end())
    {
        result = std::make_shared<Skill>(*(*it).second);
    }
    else
    {
        IO::DataClient* client = Application::Instance->GetDataClient();
        std::unique_ptr<Skill> skill = std::make_unique<Skill>();
        skill->data_.index = index;
        if (!client->Read(skill->data_))
            return std::shared_ptr<Skill>();
        result = std::make_shared<Skill>(*skill);
        // Move to cache
        skillCache_.emplace(index, std::move(skill));
    }

    if (result)
    {
        if (result->LoadScript(IO::DataProvider::Instance.GetDataFile(result->data_.script)))
            return result;
    }

    return std::shared_ptr<Skill>();
}

}
