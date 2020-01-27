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
#include <AB/Entities/Item.h>

using bitsery::ext::BaseClass;

namespace AB {
namespace Entities {

static constexpr auto KEY_TYPED_ITEMLIST = "typed_item_list";

struct TypedListItem
{
    std::string uuid;
    ItemType belongsTo = ItemTypeUnknown;
    float chance = 0.0f;
};

/// Item by type on a certain map with drop chances. UUID is the map uuid.
struct TypedItemList : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_TYPED_ITEMLIST;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value2b(type);
        s.container(items, Limits::MAX_ITEMS, [&s](TypedListItem& c)
        {
            s.text1b(c.uuid, Limits::MAX_UUID);
            s.value2b(c.belongsTo);
            s.value4b(c.chance);
        });
    }

    ItemType type = ItemTypeUnknown;
    std::vector<TypedListItem> items;
};

/// All insignias
struct TypedItemsInsignia : TypedItemList
{
    static constexpr const char* KEY()
    {
        return "insignia_item_list";
    }
    TypedItemsInsignia() :
        TypedItemList()
    {
        type = ItemTypeModifierInsignia;
    }
};

/// All runes
struct TypedItemsRunes : TypedItemList
{
    static constexpr const char* KEY()
    {
        return "runes_item_list";
    }
    TypedItemsRunes() :
        TypedItemList()
    {
        type = ItemTypeModifierRune;
    }
};

struct TypedItemsWeaponPrefix : TypedItemList
{
    static constexpr const char* KEY()
    {
        return "weapon_prefix_item_list";
    }
    TypedItemsWeaponPrefix() :
        TypedItemList()
    {
        type = ItemTypeModifierWeaponPrefix;
    }
};

struct TypedItemsWeaponSuffix : TypedItemList
{
    static constexpr const char* KEY()
    {
        return "weapon_suffix_item_list";
    }
    TypedItemsWeaponSuffix() :
        TypedItemList()
    {
        type = ItemTypeModifierWeaponSuffix;
    }
};

struct TypedItemsWeaponInscription : TypedItemList
{
    static constexpr const char* KEY()
    {
        return "weapon_inscrition_item_list";
    }
    TypedItemsWeaponInscription() :
        TypedItemList()
    {
        type = ItemTypeModifierWeaponInscription;
    }
};

}
}
