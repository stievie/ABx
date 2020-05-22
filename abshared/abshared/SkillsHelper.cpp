/**
 * Copyright 2020 Stefan Ascher
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


#include "SkillsHelper.h"

namespace Game {

bool SkillProfessionMatches(const AB::Entities::Skill& skill, const AB::Entities::Profession& prof1,
    const AB::Entities::Profession* prof2)
{
    if (skill.professionUuid.compare(AB::Entities::PROFESSION_NONE_UUID) == 0)
        // Available for all professions
        return true;
    if (skill.professionUuid.compare(prof1.uuid) == 0)
        return true;
    if (prof2 && prof2->index != 0)
    {
        if (skill.professionUuid.compare(prof2->uuid) == 0)
            return true;
    }
    return false;
}

}
