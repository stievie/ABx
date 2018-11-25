#pragma once

#include <AB/Entities/Profession.h>
#include "SkillBar.h"
#include <AB/CommonConfig.h>
#include <AB/TemplEncoder.h>

namespace Game {
class SkillBar;
}

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
        AB::Attributes& attributes, AB::SkillIndices& skills);
};

}
