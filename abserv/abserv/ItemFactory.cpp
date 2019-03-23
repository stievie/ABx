#include "stdafx.h"
#include "ItemFactory.h"
#include "Subsystems.h"
#include "DataClient.h"
#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include "Mechanic.h"

namespace Game {

ItemFactory::ItemFactory() = default;

std::unique_ptr<Item> ItemFactory::CreateItem(const std::string& itemUuid,
    uint32_t level /* = LEVEL_CAP */,
    const std::string& accUuid /* = Utils::Uuid::EMPTY_UUID */,
    const std::string& playerUuid /* = Utils::Uuid::EMPTY_UUID */)
{
    auto client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item gameItem;
    gameItem.uuid = itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Unable to read item with UUID " << itemUuid << std::endl;
        return std::unique_ptr<Item>();
    }

    std::unique_ptr<Item> result = std::make_unique<Item>(gameItem);
    if (!result->LoadScript(result->data_.script))
        return std::unique_ptr<Item>();

    const uuids::uuid guid = uuids::uuid_system_generator{}();
    AB::Entities::ConcreteItem ci;
    ci.uuid = guid.to_string();
    ci.itemUuid = gameItem.uuid;
    ci.accountUuid = accUuid;
    ci.playerUuid = playerUuid;
    ci.creation = Utils::Tick();
    if (!client->Create(ci))
    {
        LOG_ERROR << "Unable top create concrete item" << std::endl;
        return std::unique_ptr<Item>();
    }
    // Create item stats for this drop
    if (!result->GenerateConcrete(ci, level))
        return std::unique_ptr<Item>();
    // Save the created stats
    client->Update(result->concreteItem_);
    return result;
}

std::unique_ptr<Item> ItemFactory::LoadConcrete(const std::string& concreteUuid)
{
    AB::Entities::ConcreteItem ci;
    auto client = GetSubsystem<IO::DataClient>();
    ci.uuid = concreteUuid;
    if (!client->Read(ci))
    {
        LOG_ERROR << "Error loading concrete item " << concreteUuid << std::endl;
        return std::unique_ptr<Item>();
    }
    AB::Entities::Item gameItem;
    gameItem.uuid = ci.itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Error loading item " << ci.itemUuid << std::endl;
        return std::unique_ptr<Item>();
    }
    std::unique_ptr<Item> result = std::make_unique<Item>(gameItem);
    if (!result->LoadConcrete(ci))
        return std::unique_ptr<Item>();
    if (!result->LoadScript(result->data_.script))
        return std::unique_ptr<Item>();

    return result;
}


}
