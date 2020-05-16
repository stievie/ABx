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
#include "SkillManager.h"
#include "DataProvider.h"
#include <AB/Entities/Skill.h>

namespace Game {

SkillManager::SkillManager() = default;

ea::shared_ptr<Skill> SkillManager::Get(uint32_t index)
{
    if (index == 0)
        return ea::shared_ptr<Skill>();

    ea::shared_ptr<Skill> result;
    auto it = skillCache_.find(index);
    if (it != skillCache_.end())
    {
        result = ea::make_shared<Skill>((*it).second);
    }
    else
    {
        IO::DataClient* client = GetSubsystem<IO::DataClient>();
        AB::Entities::Skill skill;
        skill.index = index;
        if (!client->Read(skill))
        {
            LOG_ERROR << "Error reading skill with index " << index << std::endl;
            return ea::shared_ptr<Skill>();
        }
        result = ea::make_shared<Skill>(skill);
        // Move to cache
        skillCache_.emplace(index, skill);
    }

    if (result)
    {
        if (result->LoadScript(result->data_.script))
            return result;
    }

    return ea::shared_ptr<Skill>();
}

}
