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

#pragma once

#include <AB/Entities/Entity.h>
#include <vector>

namespace AB {
namespace Entities {

enum ProfessionIndex : uint32_t
{
    ProfessionIndexNone = 0,
    ProfessionIndexWarrior = 1,
    ProfessionIndexRanger = 2,
    ProfessionIndexMonk = 3,
    ProfessionIndexNecromancer = 4,
    ProfessionIndexMesmer = 5,
    ProfessionIndexElementarist = 6
};

/// Usual position of the profession. Used by the matchmaking server to make balanced teams
enum class ProfessionPosition : uint8_t
{
    None = 0,
    Frontline,                   // Synonym for Warrior
    Midline,                     // DD
    Backline,                    // Synonym for supporter, healer, prot etc.
};

struct AttriInfo
{
    std::string uuid;
    uint32_t index{ 0 };
    bool primary{ false };
};

inline constexpr auto PROFESSION_NONE_UUID = "79b75ff4-92f0-11e8-a7ca-02100700d6f0";

struct Profession : Entity
{
    MAKE_ENTITY(Profession)
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.text1b(name, Limits::MAX_PROFESSION_NAME);
        s.text1b(abbr, Limits::MAX_PROFESSION_ABBR);
        s.value4b(modelIndexFemale);
        s.value4b(modelIndexMale);
        s.value1b(position);
        s.value4b(attributeCount);
        s.container(attributes, Limits::MAX_PROFESSION_ATTRIBUTES, [&s](AttriInfo& a)
        {
            s.text1b(a.uuid, Limits::MAX_UUID);
            s.value4b(a.index);
            s.value1b(a.primary);
        });
    }

    uint32_t index = 0;
    std::string name;
    std::string abbr;
    uint32_t modelIndexFemale = 0;
    uint32_t modelIndexMale = 0;
    ProfessionPosition position = ProfessionPosition::None;
    uint32_t attributeCount = 0;
    std::vector<AttriInfo> attributes;
};

}
}
