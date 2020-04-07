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

enum ItemType : uint16_t
{
    ItemTypeUnknown = 0,
    ItemTypeCharacterModel = 1,
    ItemTypeCharacterHair,
    ItemTypePortal,
    ItemTypeAreaOfEffect,
    ItemTypeAccountChest,

    // Equipments
    ItemTypeArmorHead = 10,
    ItemTypeArmorChest,
    ItemTypeArmorHands,
    ItemTypeArmorLegs,
    ItemTypeArmorFeet,
    ItemTypeModifierInsignia,
    ItemTypeModifierRune,
    ItemTypeModifierWeaponPrefix,
    ItemTypeModifierWeaponSuffix,
    ItemTypeModifierWeaponInscription,
    // Weapons
    ItemTypeWeaponFirst = 30,
    ItemTypeAxe = ItemTypeWeaponFirst,
    ItemTypeSword,
    ItemTypeHammer,
    ItemTypeFlatbow,
    ItemTypeHornbow,
    ItemTypeShortbow,
    ItemTypeLongbow,
    ItemTypeRecurvebow,
    ItemTypeStaff,
    ItemTypeWand,
    ItemTypeDaggers,
    ItemTypeScyte,
    ItemTypeSpear,
    ItemTypeFocus,
    ItemTypeShield,
    ItemTypeWeaponLast = ItemTypeShield,
    // Items that are spawned by weapons
    ItemTypeSpawnArrow,
    ItemTypeSpawnSpear,
    ItemTypeSpawnWandStaff,
    ItemTypeSpawnSkill,                            // E.g. Meteor
    ItemTypeParticle,

    // Other
    ItemTypeMaterial = 1000,                       // Material, e.g. Iron
    ItemTypeTropy,                                 // Nothing useful

    ItemTypeMoney = 65534
};

enum ModelClass : uint32_t
{
    ModelClassUnknown = 0,

    // Character models
    ModelClassWarriorFemale,
    ModelClassWarriorMale,
    ModelClassElementaristFemale,
    ModelClassElementaristMale,
    ModelClassMesmerFemale,
    ModelClassMesmerMale,
    ModelClassNecromancerFemale,
    ModelClassNecromancerMale,
    ModelClassPriestFemale,
    ModelClassPriestMale,
    ModelClassRangerFemale,
    ModelClassRangerMale,

    ModelClassLogicStart = 100,
    ModelClassPortal,
    ModelClassAccountChest,
    // Doesn't do anything visual, e.g. no animations
    ModelClassStaticNpc,

    ModelClassItemSpawns = 500,
    ModelClassArrow,

    // AOE may have an animation and particles
    ModelClassAoe = 1000,
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
    }

    uint32_t index = INVALID_INDEX;
    ModelClass model_class = ModelClassUnknown;
    std::string name;
    std::string script;
    std::string iconFile;
    std::string objectFile;
    ItemType type = ItemTypeUnknown;
    ItemType belongsTo = ItemTypeUnknown;
    bool stackAble = false;
    uint16_t value = 0;
    std::string spawnItemUuid = EMPTY_GUID;
    std::string actorScript;
};

}
}
