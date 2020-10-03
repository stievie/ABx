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
#include <AB/Entities/ConcreteItem.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_PLAYER_ITEMLIST = "player_item_list";

template<StoragePlace _Place>
struct _PlayerItemList : Entity
{
    MAKE_ENTITY(_PlayerItemList<_Place>);
    static constexpr StoragePlace Place = _Place;
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value1b(storagePlace);
        s.container(itemUuids, Limits::MAX_ITEMS, [&s](std::string& c)
        {
            s.text1b(c, Limits::MAX_UUID);
        });
    }

    StoragePlace storagePlace = _Place;
    std::vector<std::string> itemUuids;
};

using PlayerItemList = _PlayerItemList<StoragePlace::None>;
using EquippedItems = _PlayerItemList<StoragePlace::Equipped>;
using InventoryItems = _PlayerItemList<StoragePlace::Inventory>;

}
}
