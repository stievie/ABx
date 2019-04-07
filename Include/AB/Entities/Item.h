#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_ITEMS = "game_items";

enum ItemType : uint16_t
{
    ItemTypeUnknown = 0,
    ItemTypeCharacterModel = 1,
    ItemTypeCharacterHair,
    ItemTypePortal,
    ItemTypeAreaOfEffect,

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
    ItemTypeWeaponLast = ItemTypeSpear,
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
        s.text1b(name, Limits::MAX_ITEM_NAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.text1b(server_icon, Limits::MAX_FILENAME);
        s.text1b(server_model, Limits::MAX_FILENAME);
        s.text1b(client_icon, Limits::MAX_FILENAME);
        s.text1b(client_model, Limits::MAX_FILENAME);
        s.value2b(type);
        s.value1b(stackAble);
    }

    uint32_t index = INVALID_INDEX;
    std::string name;
    std::string script;
    std::string server_icon;
    std::string server_model;
    std::string client_icon;
    std::string client_model;
    ItemType type = ItemTypeUnknown;
    bool stackAble = false;
};

}
}
