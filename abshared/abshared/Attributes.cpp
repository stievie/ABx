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


#include "Attributes.h"
#include <algorithm>
#include "AttribAlgos.h"

namespace Game {

uint32_t GetAttribRank(const Attributes& attributes, Attribute index)
{
    // This works only when professions are set, which fills the attributes array
    auto it = std::find_if(attributes.begin(), attributes.end(), [&](const AttributeValue& attrib)
    {
        return attrib.index == index;
    });
    if (it == attributes.end())
        return 0;
    return (*it).value;
}

bool SetAttribRank(Attributes& attributes, Attribute index, uint32_t value)
{
    auto it = std::find_if(attributes.begin(), attributes.end(), [&](const AttributeValue& attrib) {
        return attrib.index == index;
    });
    if (it == attributes.end())
        return false;
    if ((*it).value == value)
        return true;
    (*it).value = value;
    return true;
}

void InitProf2Attribs(Attributes& attributes, const AB::Entities::Profession& prof1, const AB::Entities::Profession* prof2)
{
    for (size_t i = prof1.attributeCount; i < attributes.size(); ++i)
    {
        auto& a = attributes[i];
        if (prof2)
        {
            if (prof2->attributes.size() > i - prof1.attributeCount)
                a.index = static_cast<Attribute>(prof2->attributes[i - prof1.attributeCount].index);
            else
                a.index = Attribute::None;
        }
        else
            a.index = Attribute::None;
        a.value = 0;
    }
}

int GetUsedAttribPoints(const Attributes& attributes, int without)
{
    int result = 0;
    for (const auto& a : attributes)
    {
        if (static_cast<int>(a.index) == without)
            continue;
        if (a.value > 0)
            result += CalcAttributeCost(static_cast<int>(a.value));
    }
    return result;
}

std::set<Attribute> GetPossibleItemAttributes(AB::Entities::ItemType type)
{
    switch (type)
    {
    case AB::Entities::ItemType::Axe:
        return { Attribute::AxeMatery };
    case AB::Entities::ItemType::Sword:
        return { Attribute::SwordsManship };
    case AB::Entities::ItemType::Wand:
    case AB::Entities::ItemType::Staff:
    case AB::Entities::ItemType::Focus:
        return { Attribute::Air,
            Attribute::Blood,
            Attribute::Curses,
            Attribute::Death,
            Attribute::DevineFavour,
            Attribute::Domination,
            Attribute::Earth,
            Attribute::EnergyStorage,
            Attribute::FastCast,
            Attribute::Fire,
            Attribute::Healing,
            Attribute::Illusion,
            Attribute::Inspiration,
            Attribute::Protection,
            Attribute::Smiting,
            Attribute::SoulReaping,
            Attribute::Water };
    case AB::Entities::ItemType::Spear:
        return { };
    case AB::Entities::ItemType::Hammer:
        return { Attribute::HammerMastery };
    case AB::Entities::ItemType::Flatbow:
    case AB::Entities::ItemType::Hornbow:
    case AB::Entities::ItemType::Shortbow:
    case AB::Entities::ItemType::Longbow:
    case AB::Entities::ItemType::Recurvebow:
        return { Attribute::MarkMansship };
    case AB::Entities::ItemType::Daggers:
        return { };
    case AB::Entities::ItemType::Scyte:
        return { };
    case AB::Entities::ItemType::Shield:
        return { Attribute::Strength, Attribute::Tactics };
    case AB::Entities::ItemType::ArmorHead:
        return {
#define ENUMERATE_ATTRIBUTE(v) Attribute::v,
    ENUMERATE_ATTRIBUTES
#undef ENUMERATE_ATTRIBUTE
        };
    case AB::Entities::ItemType::ArmorChest:
    case AB::Entities::ItemType::ArmorHands:
    case AB::Entities::ItemType::ArmorLegs:
    case AB::Entities::ItemType::ArmorFeet:
        return { };
    default:
        return { };
    }
}

}
