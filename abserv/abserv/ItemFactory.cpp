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
    result->concreteItem_.uuid = guid.to_string();
    result->concreteItem_.itemUuid = gameItem.uuid;
    result->concreteItem_.creation = Utils::Tick();
    if (!client->Create(result->concreteItem_))
    {
        LOG_ERROR << "Unable top create concrete item" << std::endl;
        return std::unique_ptr<Item>();
    }

    return result;
}


}
