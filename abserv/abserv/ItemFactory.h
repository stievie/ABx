#pragma once

#include "Item.h"
#include "UuidUtils.h"
#include "Mechanic.h"
#include "WeightedSelector.h"

namespace Game {

class ItemFactory
{
private:
    using ItemSelector = Utils::WeightedSelector<std::string>;
    std::unordered_map<std::string, std::unique_ptr<ItemSelector>> dropChances_;
    std::mutex lock_;
public:
    ItemFactory();
    ~ItemFactory() = default;

    /// Creates a new concrete item of the item and saves it to DB
    std::unique_ptr<Item> CreateItem(const std::string& itemUuid,
        uint32_t level = LEVEL_CAP,
        const std::string& accUuid = Utils::Uuid::EMPTY_UUID,
        const std::string& playerUuid = Utils::Uuid::EMPTY_UUID);
    std::unique_ptr<Item> LoadConcrete(const std::string& concreteUuid);
    /// Deletes a concrete item from the database, e.g. when an item was not picked up
    void DeleteConcrete(const std::string& uuid);
    void LoadDropChances(const std::string mapUuid);
    /// Delete drop chances for this map
    void DeleteMap(const std::string& uuid);
    std::unique_ptr<Item>CreateDropItem(const std::string& mapUuid,
        uint32_t level, const std::string& playerUuid);
};

}
