#include "stdafx.h"
#include "TemplateEncoder.h"
#include <base64.h>
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
    std::vector<uint8_t> buff;
    // 1 Byte: Header
    buff.push_back(GetSkillsTemplateHeader());
    // 2 Byte: Professions
    buff.push_back(static_cast<uint8_t>(skills.prof1_.index));
    buff.push_back(static_cast<uint8_t>(skills.prof2_.index));
    // 1 Byte: Attributes count
    const uint8_t attribCount = static_cast<uint8_t>(skills.prof1_.attributeCount + skills.prof2_.attributeCount);
    buff.push_back(attribCount);
    // Attributes
    const Game::Attributes& attribs = skills.GetAttributes();
    for (uint8_t i = 0; i < attribCount; i++)
    {
        buff.push_back(static_cast<uint8_t>(attribs[i].index));
        buff.push_back(static_cast<uint8_t>(attribs[i].value));
    }
    // Skills
    const Game::SkillsArray& _skills = skills.GetArray();
    for (const auto& s : _skills)
    {
        const uint16_t skillIndex = (s ? static_cast<uint16_t>(s->data_.index) : 0);
        const uint8_t partA = (uint8_t)((skillIndex & 0xFF00) >> 8);
        const uint8_t partB = (uint8_t)(skillIndex & 0x00FF);
        buff.push_back(partA);
        buff.push_back(partB);
    }

    return base64::encode(buff.data(), buff.size());
}

bool TemplateEncoder::Decode(const std::string& templ,
    AB::Entities::Profession& prof1, AB::Entities::Profession& prof2,
    Game::Attributes& attributes, std::array<uint32_t, PLAYER_MAX_SKILLS>& skills)
{
    if (templ.empty())
        return false;

    std::string s = base64::decode(templ);
    std::vector<uint8_t> vec(s.begin(), s.end());
    int pos = 0;
    if (vec[pos] != GetSkillsTemplateHeader())
        return false;

    ++pos;
    prof1.index = vec[pos];
    ++pos;
    prof2.index = vec[pos];
    ++pos;
    const uint8_t attribCount = vec[pos];
    ++pos;
    for (uint8_t i = 0; i < attribCount; i++)
    {
        attributes[i].index = vec[pos];
        ++pos;
        attributes[i].value = vec[pos];
        ++pos;
    }

    for (int i = 0; i < PLAYER_MAX_SKILLS; i++)
    {
        const uint8_t partA = vec[pos];
        ++pos;
        const uint8_t partB = vec[pos];
        ++pos;
        skills[i] = (partA << 8) | partB;
    }
    return true;
}

}
