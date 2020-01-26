#pragma once

#include <AB/CommonConfig.h>
#include <AB/Entities/Profession.h>
#include <vector>
#include <base64.h>
#include <array>
#include <stdint.h>

#define SKILLS_TEMPLATE_HEADER_VERSION (0)
#define SKILLS_TEMPLATE_HEADER_TYPE (0xe)

namespace AB {

struct AttributeValue
{
    uint32_t index = 99;    // No attribute
    uint32_t value = 0;     // How many points
};

inline int CalcAttributeCost(int rank)
{
    static const int cost[] = {
        1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20
    };
    int result = 0;
    for (int i = 0; i < rank; ++i)
    {
        if (i < 12)
            result += cost[i];
        else
            result += i;
    }
    return result;
}

typedef std::array<AttributeValue, Game::PLAYER_MAX_ATTRIBUTES> Attributes;
typedef std::array<uint32_t, Game::PLAYER_MAX_SKILLS> SkillIndices;

class TemplEncoder
{
public:
    static uint8_t GetSkillsTemplateHeader()
    {
        return (SKILLS_TEMPLATE_HEADER_TYPE << 4) | SKILLS_TEMPLATE_HEADER_VERSION;
    }

    static std::string Encode(const AB::Entities::Profession& prof1, const AB::Entities::Profession& prof2,
        const Attributes& attribs, const SkillIndices& skills)
    {
        std::vector<uint8_t> buff;
        // 1 Byte: Header
        buff.push_back(GetSkillsTemplateHeader());
        // 2 Byte: Professions
        buff.push_back(static_cast<uint8_t>(prof1.index));
        buff.push_back(static_cast<uint8_t>(prof2.index));
        // 1 Byte: Attributes count
        const uint8_t attribCount = static_cast<uint8_t>(prof1.attributeCount + prof2.attributeCount);
        buff.push_back(attribCount);
        // Attributes
        for (uint8_t i = 0; i < attribCount; i++)
        {
            buff.push_back(static_cast<uint8_t>(attribs[i].index));
            buff.push_back(static_cast<uint8_t>(attribs[i].value));
        }
        // Skills
        for (const auto& s : skills)
        {
            const uint16_t skillIndex = static_cast<uint16_t>(s);
            const uint8_t partA = (uint8_t)((skillIndex & 0xFF00) >> 8);
            const uint8_t partB = (uint8_t)(skillIndex & 0x00FF);
            buff.push_back(partA);
            buff.push_back(partB);
        }

        return base64::encode(buff.data(), buff.size());
    }

    static bool Decode(const std::string& templ,
        AB::Entities::Profession& prof1, AB::Entities::Profession& prof2,
        Attributes& attribs, SkillIndices& skills)
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
            attribs[i].index = vec[pos];
            ++pos;
            attribs[i].value = vec[pos];
            ++pos;
        }

        for (int i = 0; i < Game::PLAYER_MAX_SKILLS; i++)
        {
            const uint8_t partA = vec[pos];
            ++pos;
            const uint8_t partB = vec[pos];
            ++pos;
            skills[i] = (partA << 8) | partB;
        }
        return true;
    }
};

}
