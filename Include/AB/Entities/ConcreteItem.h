#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

enum StoragePlace : uint8_t
{
    StoragePlaceNone = 0,
    StoragePlaceScene = 1,      // On ground
    StoragePlaceInventory,      // In players inventory
    StoragePlaceChest,          // Account chest
    StoragePlaceEquipped,       // A player has equipped this item
    StoragePlaceMerchant,       // Merchant has this item and may be bought by a player
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
        s.value2b(value);
        s.text1b(instanceUuid, Limits::MAX_UUID);
        s.text1b(mapUuid, Limits::MAX_UUID);
    }

    std::string playerUuid = EMPTY_GUID;
    std::string accountUuid = EMPTY_GUID;
    StoragePlace storagePlace = StoragePlaceScene;
    uint16_t storagePos = 0;
    std::string upgrade1Uuid = EMPTY_GUID;
    std::string upgrade2Uuid = EMPTY_GUID;
    std::string upgrade3Uuid = EMPTY_GUID;
    std::string itemUuid = EMPTY_GUID;
    std::string itemStats;
    uint32_t count;
    int64_t creation = 0;
    uint16_t value = 0;
    std::string instanceUuid = EMPTY_GUID;
    std::string mapUuid = EMPTY_GUID;
};

}
}
