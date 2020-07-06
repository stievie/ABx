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

enum class StoragePlace : uint8_t
{
    None = 0,
    Scene = 1,      // On ground
    Inventory,      // In players inventory
    Chest,          // Account chest
    Equipped,       // A player has equipped this item
    Merchant,       // Merchant has this item and may be bought by a player
};

enum ConcreteItemFlag : uint32_t
{
    ConcreteItemFlagCustomized = 1,
};

static constexpr auto KEY_CONCRETE_ITEMS = "concrete_items";

struct ConcreteItem : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_CONCRETE_ITEMS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.text1b(playerUuid, Limits::MAX_UUID);
        s.text1b(accountUuid, Limits::MAX_UUID);
        s.text1b(upgrade1Uuid, Limits::MAX_UUID);
        s.text1b(upgrade2Uuid, Limits::MAX_UUID);
        s.text1b(upgrade3Uuid, Limits::MAX_UUID);
        s.text1b(itemUuid, Limits::MAX_UUID);
        s.text1b(itemStats, Limits::MAX_ITEM_STATS);

        s.value1b(storagePlace);
        s.value2b(storagePos);
        s.value4b(count);
        s.value8b(creation);
        s.value8b(deleted);
        s.value2b(value);
        s.text1b(instanceUuid, Limits::MAX_UUID);
        s.text1b(mapUuid, Limits::MAX_UUID);
        s.value4b(flags);
    }

    std::string playerUuid{ EMPTY_GUID };
    std::string accountUuid{ EMPTY_GUID };
    StoragePlace storagePlace{ StoragePlace::None };
    uint16_t storagePos{ 0 };
    std::string upgrade1Uuid{ EMPTY_GUID };
    std::string upgrade2Uuid{ EMPTY_GUID };
    std::string upgrade3Uuid{ EMPTY_GUID };
    std::string itemUuid{ EMPTY_GUID };
    std::string itemStats;
    uint32_t count{ 0 };
    timestamp_t creation{ 0 };
    timestamp_t deleted{ 0 };
    uint16_t value{ 0 };
    std::string instanceUuid{ EMPTY_GUID };
    std::string mapUuid{ EMPTY_GUID };
    uint32_t flags{ 0 };
    timestamp_t sold{ 0 };
};

inline bool IsItemCustomized(uint32_t flags)
{
    return (flags & ConcreteItemFlagCustomized) == ConcreteItemFlagCustomized;
}

}
}
