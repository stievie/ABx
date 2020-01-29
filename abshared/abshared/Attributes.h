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

#pragma once

#include <stdint.h>
#include <array>
#include "Mechanic.h"
#include <AB/Entities/Profession.h>

namespace Game {

enum class Attribute : uint32_t
{
    FastCast = 0,
    Illusion,
    Domination,
    Inspiration,
    Blood,
    Death,
    SoulReaping,
    Curses,
    Air,
    Earth,
    Fire,
    Water,
    EnergyStorage,
    Healing,
    Smiting,
    Protection,
    DevineFavour,
    Strength,
    AxeMatery,
    HammerMastery,
    SwordsManship,
    Tactics,
    BeastMastery,
    Expertise,
    WildernessSurvival,
    MarkMansship,
    __Last,

    None = 99
};

struct AttributeValue
{
    Attribute index{ Attribute::None };    // No attribute
    uint32_t value{ 0 };     // How many points
};

typedef std::array<AttributeValue, PLAYER_MAX_ATTRIBUTES> Attributes;
typedef std::array<uint32_t, PLAYER_MAX_SKILLS> SkillIndices;

uint32_t GetAttribVal(const Attributes& attributes, Attribute index);
bool SetAttribVal(Attributes& attributes, Attribute index, uint32_t value);
void InitProf2Attribs(Attributes& attributes, const AB::Entities::Profession& prof1, const AB::Entities::Profession* prof2);

}
