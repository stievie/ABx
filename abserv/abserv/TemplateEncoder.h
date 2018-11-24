#pragma once

#include <AB/Entities/Profession.h>
#include "SkillBar.h"

namespace Game {
class SkillBar;
}

#define SKILLS_TEMPLATE_HEADER_VERSION (0)
#define SKILLS_TEMPLATE_HEADER_TYPE (0xe)

namespace IO {

class TemplateEncoder
{
public:
    TemplateEncoder() = delete;
    ~TemplateEncoder() = delete;

    static uint8_t GetSkillsTemplateHeader();
    static std::string Encode(const Game::SkillBar& skills);

    static bool Decode(const std::string& templ,
        AB::Entities::Profession& prof1, AB::Entities::Profession& prof2,
        Game::Attributes attributes, std::array<uint32_t, PLAYER_MAX_SKILLS>& skills);
};

}
