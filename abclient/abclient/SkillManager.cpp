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


#include "SkillManager.h"

SkillManager::SkillManager(Context* context) :
    Object(context)
{
}

SkillManager::~SkillManager() = default;

AB::Entities::Profession* SkillManager::GetProfession(const String& uuid)
{
    auto it = professions_.find(std::string(uuid.CString()));
    if (it == professions_.end())
        return nullptr;
    return &(*it).second;
}

AB::Entities::AttriInfo* SkillManager::GetAttrInfo(AB::Entities::Profession& prof, const String& uuid)
{
    std::string s_uuid(uuid.CString());
    auto it = std::find_if(prof.attributes.begin(), prof.attributes.end(), [&](const AB::Entities::AttriInfo& current) -> bool {
        return s_uuid.compare(current.uuid) == 0;
    });
    if (it == prof.attributes.end())
        return nullptr;
    return &(*it);
}

const AB::Entities::Attribute* SkillManager::GetAttribute(const std::string& uuid) const
{
    for (const auto& attrib : attributes_)
    {
        if (attrib.second.uuid.compare(uuid) == 0)
            return &attrib.second;
    }
    return nullptr;
}
