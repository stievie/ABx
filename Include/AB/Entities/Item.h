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
    ModelClassMonkFemale,
    ModelClassMonkMale,
    ModelClassRangerFemale,
    ModelClassRangerMale,

    ModelClassLogicStart = 100,
    ModelClassPortal,
    ModelClassAccountChest,

    ModelClassItemSpawns = 500,
    ModelClassArrow,

    ModelClassParticle = 1000,
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
        s.text1b(server_icon, Limits::MAX_FILENAME);
        s.text1b(server_model, Limits::MAX_FILENAME);
        s.text1b(client_icon, Limits::MAX_FILENAME);
        s.text1b(client_model, Limits::MAX_FILENAME);
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
    std::string server_icon;
    std::string server_model;
    std::string client_icon;
    std::string client_model;
    ItemType type = ItemTypeUnknown;
    ItemType belongsTo = ItemTypeUnknown;
    bool stackAble = false;
    uint16_t value = 0;
    std::string spawnItemUuid = EMPTY_GUID;
    std::string actorScript;
};

}
}
