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
#include "TemplEncoder.h"
#include <base64.h>

namespace AB {

std::string TemplEncoder::Encode(const AB::Entities::Profession& prof1,
    const AB::Entities::Profession& prof2,
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

bool TemplEncoder::Decode(const std::string& templ, AB::Entities::Profession& prof1,
    AB::Entities::Profession& prof2, Attributes& attribs, SkillIndices& skills)
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

}