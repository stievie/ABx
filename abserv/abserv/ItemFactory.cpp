#include "stdafx.h"
#include "ItemFactory.h"
#include "Subsystems.h"
#include "DataClient.h"
#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include "Mechanic.h"
#include <AB/Entities/ItemChanceList.h>
#include "Random.h"
#include "MathUtils.h"
#include "AB/Entities/TypedItemList.h"

namespace Game {

ItemFactory::ItemFactory() = default;

std::unique_ptr<Item> ItemFactory::CreateItem(const std::string& itemUuid,
    uint32_t level /* = LEVEL_CAP */,
    const std::string& accUuid /* = Utils::Uuid::EMPTY_UUID */,
    const std::string& playerUuid /* = Utils::Uuid::EMPTY_UUID */)
{
    auto rng = GetSubsystem<Crypto::Random>();
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
    ci.count = 1;
    ci.creation = Utils::Tick();
    ci.value = Math::Clamp(static_cast<uint16_t>(rng->Get<int>(MIN_ITEM_VALUE, MAX_ITEM_VALUE) / level),
        static_cast<uint16_t>(5), static_cast<uint16_t>(1000));
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

void ItemFactory::IdentiyArmor(Item* item)
{
}

void ItemFactory::IdentiyLeadHandWeapon(Item* item)
{
}

void ItemFactory::IdentifyTwoHandedWeapon(Item* item)
{
}

void ItemFactory::IdentifyOffHandWeapon(Item* item)
{
}

void ItemFactory::IdentiyItem(Item* item)
{
    switch (item->data_.type)
    {
    case AB::Entities::ItemTypeArmorHead:
    case AB::Entities::ItemTypeArmorChest:
    case AB::Entities::ItemTypeArmorHands:
    case AB::Entities::ItemTypeArmorLegs:
    case AB::Entities::ItemTypeArmorFeet:
        // Armor
        IdentiyArmor(item);
        break;
    case AB::Entities::ItemTypeAxe:
    case AB::Entities::ItemTypeSword:
    case AB::Entities::ItemTypeHammer:
    case AB::Entities::ItemTypeWand:
    case AB::Entities::ItemTypeSpear:
        // Lead hand weapon
        IdentiyLeadHandWeapon(item);
        break;
    case AB::Entities::ItemTypeFlatbow:
    case AB::Entities::ItemTypeHornbow:
    case AB::Entities::ItemTypeShortbow:
    case AB::Entities::ItemTypeLongbow:
    case AB::Entities::ItemTypeRecurvebow:
    case AB::Entities::ItemTypeDaggers:
    case AB::Entities::ItemTypeScyte:
    case AB::Entities::ItemTypeStaff:
        // Two handed
        IdentifyTwoHandedWeapon(item);
        break;
    case AB::Entities::ItemTypeFocus:
    case AB::Entities::ItemTypeShield:
        // Off hand
        IdentifyOffHandWeapon(item);
        break;
    default:
        return;
    }
}

void ItemFactory::DeleteConcrete(const std::string& uuid)
{
    AB::Entities::ConcreteItem ci;
    auto client = GetSubsystem<IO::DataClient>();
    ci.uuid = uuid;
    if (!client->Delete(ci))
    {
        LOG_WARNING << "Error deleting concrete item " << uuid << std::endl;
    }
}

void ItemFactory::LoadDropChances(const std::string mapUuid)
{
    auto it = dropChances_.find(mapUuid);
    if (it != dropChances_.end())
        // Already loaded
        return;

    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    AB::Entities::ItemChanceList il;
    il.uuid = mapUuid;
    if (!client->Read(il))
    {
        LOG_ERROR << "Failed to read Item list for map " << mapUuid << std::endl;
        return;
    }

    std::unique_ptr<ItemSelector> selector = std::make_unique<ItemSelector>();
    for (const auto& i : il.items)
    {
        selector->Add(i.first, i.second);
    }
    selector->Update();

    std::lock_guard<std::mutex> lockClass(lock_);
    dropChances_.emplace(mapUuid, std::move(selector));
}

void ItemFactory::DeleteMap(const std::string& uuid)
{
    auto it = dropChances_.find(uuid);
    if (it != dropChances_.end())
        dropChances_.erase(it);
}

std::unique_ptr<Item> ItemFactory::CreateDropItem(const std::string& mapUuid, uint32_t level, const std::string& playerUuid)
{
    auto it = dropChances_.find(mapUuid);
    if (it == dropChances_.end())
        // Maybe outpost -> no drops
        return std::unique_ptr<Item>();
    if ((*it).second->Count() == 0)
        // No drops on this map :(
        return std::unique_ptr<Item>();

    auto rng = GetSubsystem<Crypto::Random>();
    const float rnd1 = rng->GetFloat();
    const float rnd2 = rng->GetFloat();
    std::string itemUuid = (*it).second->Get(rnd1, rnd2);
    if (uuids::uuid(itemUuid).nil())
        // There is a chance that nothing drops
        return std::unique_ptr<Item>();

    return CreateItem(itemUuid, level, Utils::Uuid::EMPTY_UUID, playerUuid);
}


}
