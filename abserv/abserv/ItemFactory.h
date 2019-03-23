#pragma once

#include "Item.h"
#include "UuidUtils.h"
#include "Mechanic.h"

namespace Game {

class ItemFactory
{
public:
    ItemFactory();
    ~ItemFactory() = default;

    /// Creates a new concrete item of the item and saves it to DB
    std::unique_ptr<Item> CreateItem(const std::string& itemUuid,
        uint32_t level = LEVEL_CAP,
        const std::string& accUuid = Utils::Uuid::EMPTY_UUID,
        const std::string& playerUuid = Utils::Uuid::EMPTY_UUID);
    std::unique_ptr<Item> LoadConcrete(const std::string& concreteUuid);
};

}
