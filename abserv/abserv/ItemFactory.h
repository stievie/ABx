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
    void IdentiyArmor(Item* item, Player* player);
    void IdentiyWeapon(Item* item, Player* player);
    void IdentifyOffHandWeapon(Item* item, Player* player);
    std::unique_ptr<Item> CreateModifier(AB::Entities::ItemType modType, AB::Entities::ItemType belongsTo,
        uint32_t level, const std::string& playerUuid);
public:
    ItemFactory();
    ~ItemFactory() = default;

    /// Load items that drop on all maps
    void Initialize();
    /// Creates a new concrete item of the item and saves it to DB
    std::unique_ptr<Item> CreateItem(const std::string& itemUuid,
        uint32_t level = LEVEL_CAP,
        const std::string& accUuid = Utils::Uuid::EMPTY_UUID,
        const std::string& playerUuid = Utils::Uuid::EMPTY_UUID);
    std::unique_ptr<Item> LoadConcrete(const std::string& concreteUuid);
    void IdentiyItem(Item* item, Player* player);
    /// Deletes a concrete item from the database, e.g. when an item was not picked up
    void DeleteConcrete(const std::string& uuid);
    void DeleteItem(Item* item);
    void LoadDropChances(const std::string mapUuid);
    /// Delete drop chances for this map
    void DeleteMap(const std::string& uuid);
    std::unique_ptr<Item>CreateDropItem(const std::string& mapUuid,
        uint32_t level, const std::string& playerUuid);
};

}
