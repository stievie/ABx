#pragma once

#include "Item.h"
#include "UuidUtils.h"
#include "Mechanic.h"
#include "WeightedSelector.h"

namespace Game {

class Player;

class ItemFactory
{
private:
    using ItemSelector = Utils::WeightedSelector<std::string>;
    using TypedListValue = std::pair<std::string, AB::Entities::ItemType>;
    std::map<std::string, std::unique_ptr<ItemSelector>> dropChances_;
    /// List of upgrades. Upgrades are not specific to maps
    std::map<AB::Entities::ItemType, std::vector<TypedListValue>> typedItems_;
    std::mutex lock_;
    void IdentifyArmor(Item* item, Player* player);
    void IdentifyWeapon(Item* item, Player* player);
    void IdentifyOffHandWeapon(Item* item, Player* player);
    std::unique_ptr<Item> CreateModifier(AB::Entities::ItemType modType, Item* forItem,
        uint32_t level, bool maxStats, const std::string& playerUuid);
    void CalculateValue(const AB::Entities::Item& item, uint32_t level, AB::Entities::ConcreteItem& result);
    bool CreateDBItem(AB::Entities::ConcreteItem item);
public:
    ItemFactory();
    ~ItemFactory() = default;

    /// Load items that drop on all maps
    void Initialize();
    /// Creates a new concrete item of the item and saves it to DB
    /// maxStats: When buying the item from a merchant these are always with max stats. Dropped Items have random stats.
    std::unique_ptr<Item> CreateItem(const std::string& itemUuid,
        const std::string& instanceUuid, const std::string& mapUuid,
        uint32_t level = LEVEL_CAP,
        bool maxStats = false,
        const std::string& accUuid = Utils::Uuid::EMPTY_UUID,
        const std::string& playerUuid = Utils::Uuid::EMPTY_UUID);
    std::unique_ptr<Item> LoadConcrete(const std::string& concreteUuid);
    void IdentiyItem(Item* item, Player* player);
    /// Create temporary item, does not create a concrete item.
    std::unique_ptr<Item> CreateTempItem(const std::string& itemUuid);
    /// Deletes a concrete item from the database, e.g. when an item was not picked up
    void DeleteConcrete(const std::string& uuid);
    void DeleteItem(Item* item);
    void LoadDropChances(const std::string mapUuid);
    /// Delete drop chances for this map
    void DeleteMap(const std::string& uuid);
    std::unique_ptr<Item>CreateDropItem(const std::string& instanceUuid, const std::string& mapUuid,
        uint32_t level, Player* player);
};

}
