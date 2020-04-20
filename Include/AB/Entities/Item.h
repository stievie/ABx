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
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

/*
 * Special item indices:
 *       0: No item, the first item
 * 9999999: Money, the last item
 */
static constexpr auto KEY_ITEMS = "game_items";

enum class ItemType : uint16_t
{
    Unknown = 0,
    CharacterModel = 1,
    CharacterHair,
    Portal,
    AreaOfEffect,
    AccountChest,

    // Equipments
    ArmorHead = 10,
    ArmorChest,
    ArmorHands,
    ArmorLegs,
    ArmorFeet,
    ModifierInsignia,
    ModifierRune,
    ModifierWeaponPrefix,
    ModifierWeaponSuffix,
    ModifierWeaponInscription,
    // Weapons
    __WeaponFirst = 30,
    Axe = __WeaponFirst,
    Sword,
    Hammer,
    Flatbow,
    Hornbow,
    Shortbow,
    Longbow,
    Recurvebow,
    Staff,
    Wand,
    Daggers,
    Scyte,
    Spear,
    Focus,
    Shield,
    __WeaponLast = Shield,
    // Items that are spawned by weapons
    SpawnArrow,
    SpawnSpear,
    SpawnWandStaff,
    SpawnSkill,                            // E.g. Meteor
    Particle,

    // Other
    Material = 1000,                       // Material, e.g. Iron
    Tropy,                                 // Nothing useful

    Money = 65534
};

enum class ModelClass : uint32_t
{
    Unknown = 0,

    // Character models
    WarriorFemale,
    WarriorMale,
    ElementaristFemale,
    ElementaristMale,
    MesmerFemale,
    MesmerMale,
    NecromancerFemale,
    NecromancerMale,
    PriestFemale,
    PriestMale,
    RangerFemale,
    RangerMale,

    LogicStart = 100,
    Portal,
    AccountChest,
    // Doesn't do anything visual, e.g. no animations
    StaticNpc,

    ItemSpawns = 500,
    Arrow,

    // AOE may have an animation and particles
    Aoe = 1000,
};

struct Item : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_ITEMS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.value4b(model_class);
        s.text1b(name, Limits::MAX_ITEM_NAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.text1b(iconFile, Limits::MAX_FILENAME);
        s.text1b(objectFile, Limits::MAX_FILENAME);
        s.value2b(type);
        s.value2b(belongsTo);
        s.value1b(stackAble);
        s.value2b(value);
        s.text1b(spawnItemUuid, Limits::MAX_UUID);
        s.text1b(actorScript, Limits::MAX_FILENAME);
        s.value1b(tradeAble);
    }

    uint32_t index{ INVALID_INDEX };
    ModelClass model_class{ ModelClass::Unknown };
    std::string name;
    std::string script;
    std::string iconFile;
    std::string objectFile;
    ItemType type{ ItemType::Unknown };
    ItemType belongsTo{ ItemType::Unknown };
    bool stackAble{ false };
    uint16_t value{ 0 };
    std::string spawnItemUuid{ EMPTY_GUID };
    std::string actorScript;
    bool tradeAble{ false };
};

inline bool IsArmorItem(ItemType type)
{
    return type == ItemType::ArmorHead ||
        type == ItemType::ArmorChest ||
        type == ItemType::ArmorHands ||
        type == ItemType::ArmorLegs ||
        type == ItemType::ArmorFeet;
}

inline bool IsWeaponItem(ItemType type)
{
    return type >= ItemType::__WeaponFirst && type <= ItemType::__WeaponLast;
}

}
}
