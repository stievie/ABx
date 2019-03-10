#include "stdafx.h"
#include "SkillManager.h"
#include "DataProvider.h"
#include <AB/Entities/Skill.h>
#include "DataClient.h"

namespace Game {

SkillManager::SkillManager() = default;

std::shared_ptr<Skill> SkillManager::Get(uint32_t index)
{
    if (index == 0)
        return std::shared_ptr<Skill>();

    std::shared_ptr<Skill> result;
    auto it = skillCache_.find(index);
    if (it != skillCache_.end())
    {
        result = std::make_shared<Skill>((*it).second);
    }
    else
    {
        IO::DataClient* client = GetSubsystem<IO::DataClient>();
        AB::Entities::Skill skill;
        skill.index = index;
        if (!client->Read(skill))
        {
            LOG_ERROR << "Error reading skill with index " << index << std::endl;
            return std::shared_ptr<Skill>();
        }
        result = std::make_shared<Skill>(skill);
        // Move to cache
        skillCache_.emplace(index, skill);
    }

    if (result)
    {
        if (result->LoadScript(result->data_.script))
            return result;
    }

    return std::shared_ptr<Skill>();
}

}
