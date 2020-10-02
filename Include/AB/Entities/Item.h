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

namespace AB {
namespace Entities {

/*
 * Special item indices:
 *       0: No item, the first item
 * 9999999: Money, the last item
 */

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
    Dye,
    Consumeable,
    Tropy = 1100,                          // Nothing useful

    __Last,

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

enum ItemFlag : uint32_t
{
    ItemFlagStackable = 1,
    ItemFlagIdentifyable = 1 << 1,
    ItemFlagTradeable = 1 << 2,
    ItemFlagUpgradeable = 1 << 3,
    // When you sell an item to the merchant, the merchant may resell the item to other players.
    // Items sold to the merchant without this flag, get data nirvana'd.
    ItemFlagResellable = 1 << 4,
    // Some craftsman may make a new of this one
    ItemFlagCraftable = 1 << 5,
    ItemFlagSalvageable = 1 << 6,
};

inline constexpr uint32_t MONEY_ITEM_INDEX = 9999999;
inline constexpr uint32_t SALVAGE_KIT_ITEM_INDEX = 100112;

struct Item : Entity
{
    MAKE_ENTITY(Item)
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
        s.value2b(value);
        s.text1b(spawnItemUuid, Limits::MAX_UUID);
        s.text1b(actorScript, Limits::MAX_FILENAME);
        s.value4b(itemFlags);
    }

    uint32_t index{ INVALID_INDEX };
    ModelClass model_class{ ModelClass::Unknown };
    std::string name;
    std::string script;
    std::string iconFile;
    std::string objectFile;
    ItemType type{ ItemType::Unknown };
    ItemType belongsTo{ ItemType::Unknown };
    uint16_t value{ 0 };
    std::string spawnItemUuid{ EMPTY_GUID };
    std::string actorScript;
    uint32_t itemFlags{ 0 };
};

inline bool IsItemStackable(uint32_t flags)
{
    return (flags & ItemFlagStackable) == ItemFlagStackable;
}

inline bool IsItemIdentifyable(uint32_t flags)
{
    return (flags & ItemFlagIdentifyable) == ItemFlagIdentifyable;
}

inline bool IsItemTradeable(uint32_t flags)
{
    return (flags & ItemFlagTradeable) == ItemFlagTradeable;
}

inline bool IsItemResellable(uint32_t flags)
{
    return (flags & ItemFlagResellable) == ItemFlagResellable;
}

inline bool IsItemUpgradeable(uint32_t flags)
{
    return (flags & ItemFlagUpgradeable) == ItemFlagUpgradeable;
}

inline bool IsItemCraftable(uint32_t flags)
{
    return (flags & ItemFlagCraftable) == ItemFlagCraftable;
}

inline bool IsItemSalvageable(uint32_t flags)
{
    return (flags & ItemFlagSalvageable) == ItemFlagSalvageable;
}

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

inline bool IsConsumeableItem(ItemType type)
{
    return type >= ItemType::Consumeable;
}

}
}
