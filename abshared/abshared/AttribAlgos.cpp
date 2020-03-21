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
#include "AttribAlgos.h"
#include "Mechanic.h"

namespace Game {

int CalcAttributeCost(int rank)
{
    static const int cost[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20
    };
    int result = 0;
    for (int i = 1; i <= rank; ++i)
    {
        if (i <= 12)
            result += cost[i];
        else
            result += i;
    }
    return result;
}

uint32_t GetAttribPoints(uint32_t level)
{
    auto clamp = [](uint32_t v, uint32_t min, uint32_t max) -> uint32_t
    {
        if (v < min)
            return min;
        if (v > max)
            return max;
        return v;
    };

    uint32_t result{ 0 };
    if (level > 1)
        result += ADVANCE_ATTRIB_2_10 * (clamp(level, 2, 10) - 1);
    if (level > 10)
        result += ADVANCE_ATTRIB_11_15 * (clamp(level, 11, 15) - 10);
    if (level > 15)
        result += ADVANCE_ATTRIB_16_20 * (clamp(level, 16, 20) - 15);
    if (level >= LEVEL_CAP)
        result += 30;
    if (level > LEVEL_CAP)
        result += ADVANCE_ATTRIB_16_20 * (20 - level);
    return result;
}

}
