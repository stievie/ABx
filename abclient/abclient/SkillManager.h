#pragma once

#include <map>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Skill.h>
#include <AB/Entities/Effect.h>

class SkillManager : public Object
{
    URHO3D_OBJECT(SkillManager, Object);
public:
    SkillManager(Context* context);
    ~SkillManager();

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

    std::map<uint32_t, AB::Entities::Skill> skills_;
    std::map<std::string, AB::Entities::Profession> professions_;
    std::map<uint32_t, AB::Entities::Effect> effects_;

};

