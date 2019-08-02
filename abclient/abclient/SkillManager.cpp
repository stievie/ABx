#include "stdafx.h"
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
