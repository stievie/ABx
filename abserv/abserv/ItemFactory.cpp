#include "stdafx.h"
#include "ItemFactory.h"
#include "Subsystems.h"
#include "DataClient.h"
#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>

namespace Game {

ItemFactory::ItemFactory() = default;

std::unique_ptr<Item> ItemFactory::CreateItem(const std::string& itemUuid)
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
    const uuids::uuid guid = uuids::uuid_system_generator{}();
    AB::Entities::ConcreteItem ci;
    ci.uuid = guid.to_string();
    ci.itemUuid = gameItem.uuid;
    ci.creation = Utils::Tick();
    if (!client->Create(ci))
    {
        LOG_ERROR << "Unable top create concrete item" << std::endl;
        return std::unique_ptr<Item>();
    }
    if (!result->LoadConcrete(ci))
    {
        return std::unique_ptr<Item>();
    }
    return result;
}


}
