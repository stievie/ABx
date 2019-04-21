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
#include "Player.h"

namespace Game {

ItemFactory::ItemFactory() = default;

void ItemFactory::Initialize()
{
    // Upgrades are not dependent on the map
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    {
        AB::Entities::TypedItemsInsignia insignias;
        insignias.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(insignias))
        {
            std::vector<TypedListValue> values;
            values.reserve(insignias.items.size());
            for (const auto& i : insignias.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierInsignia, values);
        }
    }

    {
        AB::Entities::TypedItemsRunes runes;
        runes.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(runes))
        {
            std::vector<TypedListValue> values;
            values.reserve(runes.items.size());
            for (const auto& i : runes.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierRune, values);
        }
    }

    {
        AB::Entities::TypedItemsWeaponPrefix prefixes;
        prefixes.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(prefixes))
        {
            std::vector<TypedListValue> values;
            values.reserve(prefixes.items.size());
            for (const auto& i : prefixes.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierWeaponPrefix, values);
        }
    }

    {
        AB::Entities::TypedItemsWeaponSuffix suffixes;
        suffixes.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(suffixes))
        {
            std::vector<TypedListValue> values;
            values.reserve(suffixes.items.size());
            for (const auto& i : suffixes.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierWeaponSuffix, values);
        }
    }

    {
        AB::Entities::TypedItemsWeaponInscription inscriptions;
        inscriptions.uuid = Utils::Uuid::EMPTY_UUID;
        if (client->Read(inscriptions))
        {
            std::vector<TypedListValue> values;
            values.reserve(inscriptions.items.size());
            for (const auto& i : inscriptions.items)
            {
                values.push_back(std::make_pair(i.uuid, i.belongsTo));
            }
            typedItems_.emplace(AB::Entities::ItemTypeModifierWeaponInscription, values);
        }
    }
}

void ItemFactory::CalculateValue(const AB::Entities::Item& item, uint32_t level, AB::Entities::ConcreteItem& result)
{
    auto rng = GetSubsystem<Crypto::Random>();
    if (item.type != AB::Entities::ItemTypeMoney && item.type != AB::Entities::ItemTypeMaterial)
    {
        result.count = 1;
        if (item.value == 0)
            result.value = Math::Clamp(static_cast<uint16_t>(rng->Get<int>(MIN_ITEM_VALUE, MAX_ITEM_VALUE) / level),
                static_cast<uint16_t>(5), static_cast<uint16_t>(1000));
        else
            result.value = item.value;
    }
    else
    {
        // Money or Material
        result.count = Math::Clamp(rng->Get(MIN_ITEM_VALUE, MAX_ITEM_VALUE) / level, 5u, 1000u);
        if (item.type == AB::Entities::ItemTypeMoney)
            // Money
            result.value = 1;
        else
            // The value in the items table is used for materials
            result.value = item.value;
    }
}

std::unique_ptr<Item> ItemFactory::CreateItem(const std::string& itemUuid,
    const std::string& instanceUuid, const std::string& mapUuid,
    uint32_t level /* = LEVEL_CAP */,
    bool maxStats /* = false */,
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
    ci.instanceUuid = instanceUuid;
    ci.mapUuid = mapUuid;
    ci.creation = Utils::Tick();
    CalculateValue(gameItem, level, ci);

    if (!client->Create(ci))
    {
        LOG_ERROR << "Unable top create concrete item" << std::endl;
        return std::unique_ptr<Item>();
    }
    // Create item stats for this drop
    if (!result->GenerateConcrete(ci, level, maxStats))
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

void ItemFactory::IdentifyArmor(Item* item, Player* player)
{
    std::unique_ptr<Item> insignia = CreateModifier(AB::Entities::ItemTypeModifierInsignia, item,
        player->GetLevel(), false, player->data_.uuid);
    if (insignia)
        item->SetUpgrade(ItemUpgrade::Pefix, std::move(insignia));
    std::unique_ptr<Item> rune = CreateModifier(AB::Entities::ItemTypeModifierRune, item,
        player->GetLevel(), false, player->data_.uuid);
    if (rune)
        item->SetUpgrade(ItemUpgrade::Suffix, std::move(rune));
}

void ItemFactory::IdentifyWeapon(Item* item, Player* player)
{
    std::unique_ptr<Item> prefix = CreateModifier(AB::Entities::ItemTypeModifierWeaponPrefix, item,
        player->GetLevel(), false, player->data_.uuid);
    if (prefix)
        item->SetUpgrade(ItemUpgrade::Pefix, std::move(prefix));
    std::unique_ptr<Item> suffix = CreateModifier(AB::Entities::ItemTypeModifierWeaponSuffix, item,
        player->GetLevel(), false, player->data_.uuid);
    if (suffix)
        item->SetUpgrade(ItemUpgrade::Suffix, std::move(suffix));
    std::unique_ptr<Item> inscr = CreateModifier(AB::Entities::ItemTypeModifierWeaponInscription, item,
        player->GetLevel(), false, player->data_.uuid);
    if (inscr)
        item->SetUpgrade(ItemUpgrade::Inscription, std::move(inscr));
}

void ItemFactory::IdentifyOffHandWeapon(Item* item, Player* player)
{
    // Offhead weapons do not have a prefix
    std::unique_ptr<Item> suffix = CreateModifier(AB::Entities::ItemTypeModifierWeaponSuffix, item,
        player->GetLevel(), false, player->data_.uuid);
    if (suffix)
        item->SetUpgrade(ItemUpgrade::Suffix, std::move(suffix));
    std::unique_ptr<Item> inscr = CreateModifier(AB::Entities::ItemTypeModifierWeaponInscription, item,
        player->GetLevel(), false, player->data_.uuid);
    if (inscr)
        item->SetUpgrade(ItemUpgrade::Inscription, std::move(inscr));
}

std::unique_ptr<Item> ItemFactory::CreateModifier(AB::Entities::ItemType modType, Item* forItem,
    uint32_t level, bool maxStats, const std::string& playerUuid)
{
    auto it = typedItems_.find(modType);
    if (it == typedItems_.end())
        return std::unique_ptr<Item>();
    std::vector<std::vector<TypedListValue>::iterator> result;
    Utils::SelectIterators((*it).second.begin(), (*it).second.end(),
        std::back_inserter(result),
        [forItem](const TypedListValue& current)
    {
        if (!forItem->IsArmor())
            return (current.second == forItem->data_.type);
        // Armor can have only runes and insignias
        return current.second == AB::Entities::ItemTypeModifierRune ||
            current.second == AB::Entities::ItemTypeModifierInsignia;
    });

    if (result.size() == 0)
        return std::unique_ptr<Item>();
    auto selIt = Utils::SelectRandomly(result.begin(), result.end());
    if (selIt == result.end())
        return std::unique_ptr<Item>();

    return CreateItem((*(*selIt)).first, 
        forItem->concreteItem_.instanceUuid, forItem->concreteItem_.mapUuid, 
        level, maxStats, Utils::Uuid::EMPTY_UUID, playerUuid);
}

void ItemFactory::IdentiyItem(Item* item, Player* player)
{
    assert(item);
    switch (item->data_.type)
    {
    case AB::Entities::ItemTypeArmorHead:
    case AB::Entities::ItemTypeArmorChest:
    case AB::Entities::ItemTypeArmorHands:
    case AB::Entities::ItemTypeArmorLegs:
    case AB::Entities::ItemTypeArmorFeet:
        // Armor
        IdentifyArmor(item, player);
        break;
    case AB::Entities::ItemTypeAxe:
    case AB::Entities::ItemTypeSword:
    case AB::Entities::ItemTypeHammer:
    case AB::Entities::ItemTypeWand:
    case AB::Entities::ItemTypeSpear:
        // Lead hand weapon
    case AB::Entities::ItemTypeFlatbow:
    case AB::Entities::ItemTypeHornbow:
    case AB::Entities::ItemTypeShortbow:
    case AB::Entities::ItemTypeLongbow:
    case AB::Entities::ItemTypeRecurvebow:
    case AB::Entities::ItemTypeDaggers:
    case AB::Entities::ItemTypeScyte:
    case AB::Entities::ItemTypeStaff:
        // Two handed
        IdentifyWeapon(item, player);
        break;
    case AB::Entities::ItemTypeFocus:
    case AB::Entities::ItemTypeShield:
        // Off hand
        IdentifyOffHandWeapon(item, player);
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

void ItemFactory::DeleteItem(Item* item)
{
    if (!item)
        return;
    {
        auto upg = item->GetUpgrade(ItemUpgrade::Pefix);
        if (upg)
        {
            DeleteConcrete(upg->concreteItem_.uuid);
            item->RemoveUpgrade(ItemUpgrade::Pefix);
        }
    }
    {
        auto upg = item->GetUpgrade(ItemUpgrade::Suffix);
        if (upg)
        {
            DeleteConcrete(upg->concreteItem_.uuid);
            item->RemoveUpgrade(ItemUpgrade::Suffix);
        }
    }
    {
        auto upg = item->GetUpgrade(ItemUpgrade::Inscription);
        if (upg)
        {
            DeleteConcrete(upg->concreteItem_.uuid);
            item->RemoveUpgrade(ItemUpgrade::Inscription);
        }
    }
    DeleteConcrete(item->concreteItem_.uuid);
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

    {
        std::lock_guard<std::mutex> lockClass(lock_);
        dropChances_.emplace(mapUuid, std::move(selector));
    }
}

void ItemFactory::DeleteMap(const std::string& uuid)
{
    auto it = dropChances_.find(uuid);
    if (it != dropChances_.end())
        dropChances_.erase(it);
}

std::unique_ptr<Item> ItemFactory::CreateDropItem(const std::string& instanceUuid, const std::string& mapUuid, 
    uint32_t level, const std::string& playerUuid)
{
    auto it = dropChances_.find(mapUuid);
    if (it == dropChances_.end())
        // Maybe outpost -> no drops
        return std::unique_ptr<Item>();
    if (!(*it).second || (*it).second->Count() == 0)
        // No drops on this map :(
        return std::unique_ptr<Item>();

    auto rng = GetSubsystem<Crypto::Random>();
    const float rnd1 = rng->GetFloat();
    const float rnd2 = rng->GetFloat();
    const std::string& itemUuid = (*it).second->Get(rnd1, rnd2);
    if (uuids::uuid(itemUuid).nil())
        // There is a chance that nothing drops
        return std::unique_ptr<Item>();

    return CreateItem(itemUuid, instanceUuid, mapUuid, level, false, Utils::Uuid::EMPTY_UUID, playerUuid);
}

}
