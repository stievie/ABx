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
#include "UuidUtils.h"
#include "Mechanic.h"
#include <sa/WeightedSelector.h>

namespace Game {

static constexpr const char* MONEY_ITEM_UUID = "08fbf9bd-b84f-412f-ae4a-bc499784fadf";

class Player;

class ItemFactory
{
private:
    using ItemSelector = sa::WeightedSelector<std::string>;
    using TypedListValue = std::pair<std::string, AB::Entities::ItemType>;
    std::map<std::string, std::unique_ptr<ItemSelector>> dropChances_;
    /// List of upgrades. Upgrades are not specific to maps
    std::map<AB::Entities::ItemType, std::vector<TypedListValue>> typedItems_;
    std::mutex lock_;
    void IdentifyArmor(Item* item, Player* player);
    void IdentifyWeapon(Item* item, Player* player);
    void IdentifyOffHandWeapon(Item* item, Player* player);
    uint32_t CreateModifier(AB::Entities::ItemType modType, Item* forItem,
        uint32_t level, bool maxStats, const std::string& playerUuid);
    void CalculateValue(const AB::Entities::Item& item, uint32_t level, AB::Entities::ConcreteItem& result);
    bool CreateDBItem(AB::Entities::ConcreteItem item);
    std::unique_ptr<Item> LoadConcrete(const std::string& concreteUuid);
public:
    ItemFactory();
    ~ItemFactory() = default;

    /// Load items that drop on all maps
    void Initialize();
    /// Creates a new concrete item of the item and saves it to DB
    /// maxStats: When buying the item from a merchant these are always with max stats. Dropped Items have random stats.
    uint32_t CreateItem(const std::string& itemUuid,
        const std::string& instanceUuid, const std::string& mapUuid,
        uint32_t level = LEVEL_CAP,
        bool maxStats = false,
        const std::string& accUuid = Utils::Uuid::EMPTY_UUID,
        const std::string& playerUuid = Utils::Uuid::EMPTY_UUID);
    uint32_t GetConcreteId(const std::string& concreteUuid);
    void IdentiyItem(Item* item, Player* player);
    /// Create temporary item, does not create a concrete item.
    std::unique_ptr<Item> CreateTempItem(const std::string& itemUuid);
    /// Deletes a concrete item from the database, e.g. when an item was not picked up. Also removes it from cache.
    void DeleteConcrete(const std::string& uuid);
    /// Deletes an Item with all attached modifiers. Removes them from cache
    void DeleteItem(Item* item);
    void LoadDropChances(const std::string mapUuid);
    /// Delete drop chances for this map
    void DeleteMap(const std::string& uuid);
    uint32_t CreateDropItem(const std::string& instanceUuid, const std::string& mapUuid,
        uint32_t level, Player* player);
    uint32_t CreatePlayerMoneyItem(Player& forPlayer, uint32_t count);
    uint32_t CreatePlayerItem(Player& forPlayer, const std::string& itemUuid, uint32_t count = 1);
};

}
