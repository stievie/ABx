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
#include "TemplateEncoder.h"
#include "SkillBar.h"

namespace IO {

uint8_t TemplateEncoder::GetSkillsTemplateHeader()
{
    return (SKILLS_TEMPLATE_HEADER_TYPE << 4) | SKILLS_TEMPLATE_HEADER_VERSION;
}

std::string TemplateEncoder::Encode(const Game::SkillBar& skills)
{
    const Game::SkillsArray& _skills = skills.GetArray();
    Game::SkillIndices sis;
    size_t i = 0;
    for (const auto& s : _skills)
    {
        sis[i] = (s ? s->data_.index : 0);
        ++i;
    }

    return TemplEncoder::Encode(skills.prof1_, skills.prof2_, skills.GetAttributes(), sis);
}

bool TemplateEncoder::Decode(const std::string& templ,
    AB::Entities::Profession& prof1, AB::Entities::Profession& prof2,
    Game::Attributes& attributes, Game::SkillIndices& skills)
{
    return TemplEncoder::Decode(templ, prof1, prof2, attributes, skills);
}

}
