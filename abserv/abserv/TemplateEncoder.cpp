#include "stdafx.h"
#include "TemplateEncoder.h"
#include "SkillBar.h"

namespace IO {

uint8_t TemplateEncoder::GetSkillsTemplateHeader()
{
    return (SKILLS_TEMPLATE_HEADER_TYPE << 4) | SKILLS_TEMPLATE_HEADER_VERSION;
}

// https://wiki.guildwars.com/wiki/Skill_template_format
// https://wiki.guildwars.com/wiki/Equipment_template_format
// OgUUcRrg1MT6WOBqGIG/aKHXi+G
std::string TemplateEncoder::Encode(const Game::SkillBar& skills)
{
    const Game::SkillsArray& _skills = skills.GetArray();
    AB::SkillIndices sis;
    int i = 0;
    for (const auto& s : _skills)
    {
        sis[i] = (s ? s->data_.index : 0);
        ++i;
    }

    return AB::TemplEncoder::Encode(skills.prof1_, skills.prof2_, skills.GetAttributes(), sis);
}

bool TemplateEncoder::Decode(const std::string& templ,
    AB::Entities::Profession& prof1, AB::Entities::Profession& prof2,
    AB::Attributes& attributes, AB::SkillIndices& skills)
{
    return AB::TemplEncoder::Decode(templ, prof1, prof2, attributes, skills);
}

}
