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

#pragma once

#include <map>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Skill.h>
#include <AB/Entities/Effect.h>
#include <AB/Entities/Attribute.h>
#include <Urho3DAll.h>
#include <sa/Iteration.h>

class SkillManager : public Object
{
    URHO3D_OBJECT(SkillManager, Object)
public:
    SkillManager(Context* context);
    ~SkillManager() override;

    const std::map<std::string, AB::Entities::Profession>& GetProfessions() const
    {
        return professions_;
    }
    AB::Entities::Profession* GetProfessionByIndex(uint32_t index)
    {
        auto it = std::find_if(professions_.begin(), professions_.end(), [&index](const auto& current) -> bool
        {
            return current.second.index == index;
        });
        if (it != professions_.end())
            return &(*it).second;
        static AB::Entities::Profession EMPTY;
        return &EMPTY;
    }
    AB::Entities::Profession* GetProfession(const String& uuid);
    AB::Entities::AttriInfo* GetAttrInfo(AB::Entities::Profession& prof, const String& uuid);
    const AB::Entities::Attribute* GetAttributeByIndex(uint32_t index) const
    {
        auto it = attributes_.find(index);
        if (it == attributes_.end())
            return nullptr;
        return &(*it).second;
    }

    const std::map<uint32_t, AB::Entities::Skill>& GetSkills() const { return skills_; }
    const AB::Entities::Skill* GetSkillByIndex(uint32_t index) const
    {
        auto it = skills_.find(index);
        if (it == skills_.end())
            return nullptr;
        return &(*it).second;
    }
    const AB::Entities::Effect* GetEffectByIndex(uint32_t index) const
    {
        auto it = effects_.find(index);
        if (it == effects_.end())
            return nullptr;
        return &(*it).second;
    }

    template <typename Callable>
    void VisistSkillsByProfession(const std::string& profUuid, const Callable& func)
    {
        std::vector<const AB::Entities::Skill*> sorted;
        for (const auto& s : skills_)
        {
            if (s.second.index != 0 && s.second.professionUuid.compare(profUuid) == 0)
                sorted.push_back(&s.second);
        }
        std::sort(sorted.begin(), sorted.end(), [this](const auto* lhs, const auto* rhs)
        {
            const AB::Entities::Attribute* a1 = GetAttribute(lhs->attributeUuid);
            const AB::Entities::Attribute* a2 = GetAttribute(rhs->attributeUuid);
            if (a1 && a2)
            {
                int c = a1->name.compare(a2->name);
                if (c != 0)
                    return c < 0;
            }
            return lhs->name.compare(rhs->name) < 0;
        });
        for (const auto& s : sorted)
        {
            if (func(*s) == Iteration::Break)
                break;
        }
    }
    const AB::Entities::Attribute* GetAttribute(const std::string& uuid) const;

    std::map<uint32_t, AB::Entities::Skill> skills_;
    std::map<std::string, AB::Entities::Profession> professions_;
    std::map<uint32_t, AB::Entities::Attribute> attributes_;
    std::map<uint32_t, AB::Entities::Effect> effects_;
};

