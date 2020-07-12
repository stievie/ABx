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

#include <mutex>
#include "Item.h"
#include <abscommon/UuidUtils.h>
#include <abshared/Mechanic.h>
#include <sa/WeightedSelector.h>
#include <eastl.hpp>

namespace Game {

class Player;

struct CreateItemInfo
{
    std::string itemUuid;
    std::string instanceUuid;
    std::string mapUuid;
    uint32_t level{ LEVEL_CAP };
    /// When buying the item from a merchant these are always with max stats. Dropped Items have random stats.
    bool maxStats{ false };
    std::string accUuid{ Utils::Uuid::EMPTY_UUID };
    std::string playerUuid{ Utils::Uuid::EMPTY_UUID };
    uint32_t count{ 0 };
    uint16_t value{ 0 };
    AB::Entities::StoragePlace storagePlace{ AB::Entities::StoragePlace::Scene };
    std::string stats{ "" };
};

class ItemFactory
{
private:
    using ItemSelector = sa::WeightedSelector<std::string>;
    using TypedListValue = std::pair<std::string, AB::Entities::ItemType>;
    std::map<std::string, std::unique_ptr<ItemSelector>> dropChances_;
    /// List of upgrades. Upgrades are not specific to maps
    std::map<AB::Entities::ItemType, std::vector<TypedListValue>> typedItems_;
    std::mutex lock_;
    ea::map<std::string, AB::Entities::ConcreteItem> pendingCreates_;
    ea::map<ea::pair<size_t, uint32_t>, std::string> maxItemStats_;
    void CreatePendingItems();
    void IdentifyArmor(Item& item, Player& player);
    void IdentifyWeapon(Item& item, Player& player);
    void IdentifyOffHandWeapon(Item& item, Player& player);
    uint32_t CreateModifier(AB::Entities::ItemType modType, Item& forItem,
        uint32_t level, bool maxStats, const std::string& playerUuid);
    void CalculateValue(const AB::Entities::Item& item, uint32_t level, AB::Entities::ConcreteItem& result);
    bool CreateDBItem(const AB::Entities::ConcreteItem& item);
    ea::unique_ptr<Item> LoadConcrete(const std::string& concreteUuid);
    void InternalLoadDropChances(const std::string mapUuid, bool force);
public:
    ItemFactory();
    ~ItemFactory() = default;

    /// Load items that drop on all maps
    void Initialize();
    /// Creates a new concrete item of the item and saves it to DB
    uint32_t CreateItem(const CreateItemInfo& info);
    uint32_t GetConcreteId(const std::string& concreteUuid);
    void IdentiyItem(Item& item, Player& player);
    /// Create temporary item, does not create a concrete item.
    ea::unique_ptr<Item> CreateTempItem(const std::string& itemUuid);
    /// Deletes a concrete item from the database, e.g. when an item was not picked up. Also removes it from cache.
    void DeleteConcrete(const std::string& uuid);
    /// Deletes an Item with all attached modifiers. Removes them from cache
    void DeleteItem(Item* item);
    /// mapUuid is not a reference because it's called asynchronously
    void LoadDropChances(const std::string mapUuid);
    void ReloadDropChances();
    /// Delete drop chances for this map
    void DeleteMap(const std::string& uuid);
    uint32_t CreateDropItem(const std::string& instanceUuid, const std::string& mapUuid,
        uint32_t level, Player* player);
    uint32_t CreatePlayerMoneyItem(const Player& forPlayer, uint32_t count);
    uint32_t CreatePlayerItem(const Player& forPlayer, const std::string& itemUuid,
        AB::Entities::StoragePlace place, uint32_t count = 1, const std::string stats = "");
    bool MoveToMerchant(Item* item, uint32_t count);
    std::string GetMaxItemStats(const std::string& itemUuid, uint32_t level);
    // Used to craft items with a certain attribute
    std::string GetMaxItemStatsWithAttribute(const std::string& itemUuid, uint32_t level,
        Attribute attrib, int attribRank);
    uint32_t GetItemIndexFromUuid(const std::string& uuid);
};

}
