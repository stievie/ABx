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

namespace Game
{

#define ENUMERATE_ATTRIBUTES                \
    ENUMERATE_ATTRIBUTE(FastCast)           \
    ENUMERATE_ATTRIBUTE(Illusion)           \
    ENUMERATE_ATTRIBUTE(Domination)         \
    ENUMERATE_ATTRIBUTE(Inspiration)        \
    ENUMERATE_ATTRIBUTE(Blood)              \
    ENUMERATE_ATTRIBUTE(Death)              \
    ENUMERATE_ATTRIBUTE(SoulReaping)        \
    ENUMERATE_ATTRIBUTE(Curses)             \
    ENUMERATE_ATTRIBUTE(Air)                \
    ENUMERATE_ATTRIBUTE(Earth)              \
    ENUMERATE_ATTRIBUTE(Fire)               \
    ENUMERATE_ATTRIBUTE(Water)              \
    ENUMERATE_ATTRIBUTE(EnergyStorage)      \
    ENUMERATE_ATTRIBUTE(Healing)            \
    ENUMERATE_ATTRIBUTE(Smiting)            \
    ENUMERATE_ATTRIBUTE(Protection)         \
    ENUMERATE_ATTRIBUTE(DevineFavour)       \
    ENUMERATE_ATTRIBUTE(Strength)           \
    ENUMERATE_ATTRIBUTE(AxeMatery)          \
    ENUMERATE_ATTRIBUTE(HammerMastery)      \
    ENUMERATE_ATTRIBUTE(SwordsManship)      \
    ENUMERATE_ATTRIBUTE(Tactics)            \
    ENUMERATE_ATTRIBUTE(BeastMastery)       \
    ENUMERATE_ATTRIBUTE(Expertise)          \
    ENUMERATE_ATTRIBUTE(WildernessSurvival) \
    ENUMERATE_ATTRIBUTE(MarkMansship)

enum class Attribute : uint32_t
{
#define ENUMERATE_ATTRIBUTE(v) v,
    ENUMERATE_ATTRIBUTES
#undef ENUMERATE_ATTRIBUTE
    __Last,

    None = 99
};

struct AttributeValue
{
    Attribute index{ Attribute::None }; // No attribute
    uint32_t value{ 0 };                // How many points
};

typedef std::array<AttributeValue, PLAYER_MAX_ATTRIBUTES> Attributes;
typedef std::array<uint32_t, PLAYER_MAX_SKILLS> SkillIndices;

uint32_t GetAttribRank(const Attributes& attributes, Attribute index);
bool SetAttribRank(Attributes& attributes, Attribute index, uint32_t value);
void InitProf2Attribs(Attributes& attributes, const AB::Entities::Profession& prof1, const AB::Entities::Profession* prof2);
int GetUsedAttribPoints(const Attributes& attributes, int without = -1);

}
