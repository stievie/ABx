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
//include inheritance extension
//this header contains two extensions, that specifies inheritance type of base class
//  BaseClass - normal inheritance
//  VirtualBaseClass - when virtual inheritance is used
//in order for virtual inheritance to work, InheritanceContext is required.
//it can be created either internally (via configuration) or externally (pointer to context).
#include <bitsery/ext/inheritance.h>
#include <AB/Entities/Limits.h>
#include <vector>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

// Player inventory size
static constexpr size_t DEFAULT_INVENTORY_SIZE = 40;

enum class CharacterSex : uint8_t
{
    Unknown,
    Female,
    Male
};

enum DeathStatIndex : size_t
{
    DeathStatIndexCount,
    DeathStatIndexAtXp,
    __DeathStatIndexCount
};

static constexpr auto KEY_CHARACTERS = "characters";

struct Character : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_CHARACTERS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(name, Limits::MAX_CHARACTER_NAME);
        s.text1b(profession, Limits::MAX_CHARACTER_PROF);
        s.text1b(profession2, Limits::MAX_CHARACTER_PROF);
        s.text1b(professionUuid, Limits::MAX_UUID);
        s.text1b(profession2Uuid, Limits::MAX_UUID);
        s.value1b(level);
        s.value1b(pvp);
        s.value4b(xp);
        s.value4b(skillPoints);
        s.value1b(sex);
        s.text1b(accountUuid, Limits::MAX_UUID);
        s.value4b(modelIndex);
        s.text1b(skillTemplate, Limits::MAX_CHARACTER_SKILLTEMPLATE);
        s.value8b(onlineTime);
        s.value8b(deletedTime);
        s.value8b(creation);
        s.text1b(currentMapUuid, Limits::MAX_UUID);
        s.text1b(lastOutpostUuid, Limits::MAX_UUID);

        s.value8b(lastLogin);
        s.value8b(lastLogout);
        s.text1b(instanceUuid, Limits::MAX_UUID);
        s.text1b(partyUuid, Limits::MAX_UUID);
        s.value2b(inventorySize);
        s.text1b(deathStats, Limits::MAX_DEATH_STATS);
    }

    std::string name;
    std::string profession;
    std::string profession2;
    std::string professionUuid{ EMPTY_GUID };
    std::string profession2Uuid{ EMPTY_GUID };
    uint8_t level{ 0 };
    /// PvP only character
    bool pvp{ false };
    uint32_t xp{ 0 };
    uint32_t skillPoints{ 0 };
    CharacterSex sex{ CharacterSex::Unknown };
    std::string currentMapUuid{ EMPTY_GUID };
    std::string lastOutpostUuid{ EMPTY_GUID };
    std::string accountUuid{ EMPTY_GUID };
    // Index in game_items
    uint32_t modelIndex{ 0 };
    std::string skillTemplate;

    int64_t onlineTime{ 0 };
    /// 0 if not deleted
    timestamp_t deletedTime{ 0 };
    timestamp_t creation{ 0 };

    timestamp_t lastLogin{ 0 };
    timestamp_t lastLogout{ 0 };
    /// ID of AB::Entities::GameInstance
    std::string instanceUuid{ EMPTY_GUID };
    /// AB::Entities::Party
    std::string partyUuid{ EMPTY_GUID };
    uint16_t inventorySize{ DEFAULT_INVENTORY_SIZE };
    std::string deathStats;
};

typedef std::vector<Character> CharList;

}
}
