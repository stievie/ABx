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


#include "ItemFactory.h"
#include <AB/Entities/Item.h>
#include <AB/Entities/ConcreteItem.h>
#include <AB/Entities/ItemChanceList.h>
#include <AB/Entities/MerchantItem.h>
#include <AB/Entities/MerchantItemList.h>
#include <AB/Entities/TypedItemList.h>
#include "Player.h"
#include "ItemsCache.h"
#include <sa/EAIterator.h>
#include <sa/Iterator.h>
#include <sa/Transaction.h>
#include <sa/Assert.h>
#include <sa/StringHash.h>

namespace Game {

static constexpr const char* MONEY_ITEM_UUID = "08fbf9bd-b84f-412f-ae4a-bc499784fadf";

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
            typedItems_.emplace(AB::Entities::ItemType::ModifierInsignia, values);
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
            typedItems_.emplace(AB::Entities::ItemType::ModifierRune, values);
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
            typedItems_.emplace(AB::Entities::ItemType::ModifierWeaponPrefix, values);
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
            typedItems_.emplace(AB::Entities::ItemType::ModifierWeaponSuffix, values);
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
            typedItems_.emplace(AB::Entities::ItemType::ModifierWeaponInscription, values);
        }
    }
}

void ItemFactory::CalculateValue(const AB::Entities::Item& item, uint32_t level, AB::Entities::ConcreteItem& result)
{
    ASSERT(level <= LEVEL_CAP);
    const uint32_t l = (LEVEL_CAP + 1) - level;

    auto* rng = GetSubsystem<Crypto::Random>();
    if (item.type != AB::Entities::ItemType::Money && item.type != AB::Entities::ItemType::Material)
    {
        result.count = 1;
        if (item.value == 0)
            result.value = Math::Clamp(static_cast<uint16_t>(rng->Get<uint16_t>(MIN_ITEM_VALUE, MAX_ITEM_VALUE) / l),
                static_cast<uint16_t>(5), static_cast<uint16_t>(1000));
        else
            result.value = item.value;
    }
    else
    {
        // Money or Material
        if (item.type == AB::Entities::ItemType::Money)
            // Money
            result.value = 1;
        else
            // The value in the items table is used for materials
            result.value = (item.value != 0) ? item.value : 1u;
        // Count also depends on the value of the item
        if (AB::Entities::IsItemStackable(item.itemFlags))
            // Only stackable items drop count > 1. And always <= MAX_INVENTORY_STACK_SIZE
            result.count = Math::Clamp((rng->Get(MIN_ITEM_VALUE, MAX_ITEM_VALUE) / l) / result.value,
                1u, MAX_INVENTORY_STACK_SIZE);
        else
            result.count = 1;
    }
}

bool ItemFactory::CreateDBItem(const AB::Entities::ConcreteItem& item)
{
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Create(item))
    {
        LOG_ERROR << "Unable to create concrete item" << std::endl;
        return false;
    }
    return true;
}

ea::unique_ptr<Item> ItemFactory::CreateTempItem(const std::string& itemUuid)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item gameItem;
    gameItem.uuid = itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Unable to read item with UUID " << itemUuid << std::endl;
        return ea::unique_ptr<Item>();
    }

    ea::unique_ptr<Item> result = ea::make_unique<Item>(gameItem);
    if (!result->LoadScript(result->data_.script))
        return ea::unique_ptr<Item>();
    return result;
}

uint32_t ItemFactory::CreateItem(const CreateItemInfo& info)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item gameItem;
    gameItem.uuid = info.itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Unable to read item with UUID " << info.itemUuid << std::endl;
        return 0;
    }

    ea::unique_ptr<Item> result = ea::make_unique<Item>(gameItem);
    if (!result->LoadScript(result->data_.script))
    {
        LOG_ERROR << "Error loading item script " << result->data_.script << std::endl;
        return 0;
    }

    AB::Entities::ConcreteItem ci;
    ci.uuid = Utils::Uuid::New();
    ci.itemUuid = gameItem.uuid;
    ci.accountUuid = info.accUuid;
    ci.playerUuid = info.playerUuid;
    ci.instanceUuid = info.instanceUuid;
    ci.mapUuid = info.mapUuid;
    ci.creation = Utils::Tick();
    ci.storagePlace = info.storagePlace;
    CalculateValue(gameItem, info.level, ci);
    if (info.count != 0)
        ci.count = info.count;
    if (info.value != 0)
        ci.value = info.value;
    if (!AB::Entities::IsItemTradeable(gameItem.itemFlags))
        ci.value = 0;

    // Create item stats for this drop
    if (!result->GenerateConcrete(ci, info.level, info.maxStats))
    {
        LOG_ERROR << "Error generating concrete item" << std::endl;
        return 0;
    }

    pendingCreates_.emplace(result->concreteItem_.uuid, result->concreteItem_);
    // Save the created stats to the DB
    GetSubsystem<Asynch::Scheduler>()->Add(
        Asynch::CreateScheduledTask(std::bind(&ItemFactory::CreatePendingItems, this))
    );
    auto* cache = GetSubsystem<ItemsCache>();
    return cache->Add(std::move(result));
}

uint32_t ItemFactory::CreatePlayerItem(const Player& forPlayer, const std::string& itemUuid, AB::Entities::StoragePlace place, uint32_t count /* = 1 */)
{
    return CreateItem({ itemUuid,
        forPlayer.GetGame()->instanceData_.uuid,
        forPlayer.GetGame()->data_.uuid,
        forPlayer.data_.level,
        true,
        forPlayer.account_.uuid,
        forPlayer.data_.uuid, count, 0, place });
}

uint32_t ItemFactory::CreatePlayerMoneyItem(const Player& forPlayer, uint32_t count)
{
    return CreatePlayerItem(forPlayer, MONEY_ITEM_UUID, AB::Entities::StoragePlace::Inventory, count);
}

ea::unique_ptr<Item> ItemFactory::LoadConcrete(const std::string& concreteUuid)
{
    AB::Entities::ConcreteItem ci;
    auto* client = GetSubsystem<IO::DataClient>();
    ci.uuid = concreteUuid;
    if (!client->Read(ci))
    {
        LOG_ERROR << "Error loading concrete item " << concreteUuid << std::endl;
        return ea::unique_ptr<Item>();
    }
    if (ci.deleted != 0)
    {
        LOG_INFO << "Item " << concreteUuid << " was deleted" << std::endl;
        return ea::unique_ptr<Item>();
    }

    AB::Entities::Item gameItem;
    gameItem.uuid = ci.itemUuid;
    if (!client->Read(gameItem))
    {
        LOG_ERROR << "Error loading item " << ci.itemUuid << std::endl;
        return ea::unique_ptr<Item>();
    }
    ea::unique_ptr<Item> result = ea::make_unique<Item>(gameItem);
    if (!result->LoadConcrete(ci))
    {
        LOG_ERROR << "Error loading concrete item" << std::endl;
        return ea::unique_ptr<Item>();
    }
    if (!result->LoadScript(result->data_.script))
    {
        LOG_ERROR << "Error loading script " << result->data_.script << std::endl;
        return ea::unique_ptr<Item>();
    }

    return result;
}

uint32_t ItemFactory::GetConcreteId(const std::string& concreteUuid)
{
    auto* cache = GetSubsystem<ItemsCache>();
    uint32_t result = cache->GetConcreteId(concreteUuid);
    if (result != 0)
        return result;
    ea::unique_ptr<Item> item = LoadConcrete(concreteUuid);
    if (!item)
        return 0;
    return cache->Add(std::move(item));
}

void ItemFactory::CreatePendingItems()
{
    for (const auto& item : pendingCreates_)
    {
        CreateDBItem(item.second);
    }
    pendingCreates_.clear();
}

void ItemFactory::IdentifyArmor(Item& item, Player& player)
{
    uint32_t insignia = CreateModifier(AB::Entities::ItemType::ModifierInsignia, item,
        player.GetLevel(), false, player.data_.uuid);
    if (insignia != 0)
        item.SetUpgrade(ItemUpgrade::Pefix, insignia);
    uint32_t rune = CreateModifier(AB::Entities::ItemType::ModifierRune, item,
        player.GetLevel(), false, player.data_.uuid);
    if (rune != 0)
        item.SetUpgrade(ItemUpgrade::Suffix, rune);
}

void ItemFactory::IdentifyWeapon(Item& item, Player& player)
{
    uint32_t prefix = CreateModifier(AB::Entities::ItemType::ModifierWeaponPrefix, item,
        player.GetLevel(), false, player.data_.uuid);
    if (prefix != 0)
        item.SetUpgrade(ItemUpgrade::Pefix, prefix);
    uint32_t suffix = CreateModifier(AB::Entities::ItemType::ModifierWeaponSuffix, item,
        player.GetLevel(), false, player.data_.uuid);
    if (suffix != 0)
        item.SetUpgrade(ItemUpgrade::Suffix, suffix);
    uint32_t inscr = CreateModifier(AB::Entities::ItemType::ModifierWeaponInscription, item,
        player.GetLevel(), false, player.data_.uuid);
    if (inscr != 0)
        item.SetUpgrade(ItemUpgrade::Inscription, inscr);
}

void ItemFactory::IdentifyOffHandWeapon(Item& item, Player& player)
{
    // Offhead weapons do not have a prefix
    uint32_t suffix = CreateModifier(AB::Entities::ItemType::ModifierWeaponSuffix, item,
        player.GetLevel(), false, player.data_.uuid);
    if (suffix != 0)
        item.SetUpgrade(ItemUpgrade::Suffix, suffix);
    uint32_t inscr = CreateModifier(AB::Entities::ItemType::ModifierWeaponInscription, item,
        player.GetLevel(), false, player.data_.uuid);
    if (inscr != 0)
        item.SetUpgrade(ItemUpgrade::Inscription, inscr);
}

uint32_t ItemFactory::CreateModifier(AB::Entities::ItemType modType, Item& forItem,
    uint32_t level, bool maxStats, const std::string& playerUuid)
{
    auto it = typedItems_.find(modType);
    if (it == typedItems_.end())
        return 0;
    std::vector<std::vector<TypedListValue>::iterator> result;
    sa::SelectIterators((*it).second.begin(), (*it).second.end(),
        std::back_inserter(result),
        [&forItem](const TypedListValue& current)
    {
        if (!forItem.IsArmor())
            return (current.second == forItem.data_.type);
        // Armor can have only runes and insignias
        return current.second == AB::Entities::ItemType::ModifierRune ||
            current.second == AB::Entities::ItemType::ModifierInsignia;
    });

    if (result.size() == 0)
        return 0;
    auto selIt = sa::SelectRandomly(result.begin(), result.end());
    if (selIt == result.end())
        return 0;

    return CreateItem({ (*(*selIt)).first,
        forItem.concreteItem_.instanceUuid, forItem.concreteItem_.mapUuid,
        level, maxStats, Utils::Uuid::EMPTY_UUID, playerUuid, 0, 0, forItem.concreteItem_.storagePlace });
}

void ItemFactory::IdentiyItem(Item& item, Player& player)
{
    switch (item.data_.type)
    {
    case AB::Entities::ItemType::ArmorHead:
    case AB::Entities::ItemType::ArmorChest:
    case AB::Entities::ItemType::ArmorHands:
    case AB::Entities::ItemType::ArmorLegs:
    case AB::Entities::ItemType::ArmorFeet:
        // Armor
        IdentifyArmor(item, player);
        break;
    case AB::Entities::ItemType::Axe:
    case AB::Entities::ItemType::Sword:
    case AB::Entities::ItemType::Hammer:
    case AB::Entities::ItemType::Wand:
    case AB::Entities::ItemType::Spear:
        // Lead hand weapon
    case AB::Entities::ItemType::Flatbow:
    case AB::Entities::ItemType::Hornbow:
    case AB::Entities::ItemType::Shortbow:
    case AB::Entities::ItemType::Longbow:
    case AB::Entities::ItemType::Recurvebow:
    case AB::Entities::ItemType::Daggers:
    case AB::Entities::ItemType::Scyte:
    case AB::Entities::ItemType::Staff:
        // Two handed
        IdentifyWeapon(item, player);
        break;
    case AB::Entities::ItemType::Focus:
    case AB::Entities::ItemType::Shield:
        // Off hand
        IdentifyOffHandWeapon(item, player);
        break;
    default:
        return;
    }
}

void ItemFactory::DeleteConcrete(const std::string& uuid)
{
    auto* cache = GetSubsystem<ItemsCache>();

    auto it = pendingCreates_.find(uuid);
    if (it != pendingCreates_.end())
    {
        // Not yet written to DB
        pendingCreates_.erase(it);
        cache->RemoveConcrete(uuid);
        return;
    }

    AB::Entities::ConcreteItem ci;
    auto* client = GetSubsystem<IO::DataClient>();
    ci.uuid = uuid;
    // Removing from cache may invalidate the uuid argument, so do it when we don't need it anymore
    cache->RemoveConcrete(uuid);
    if (!client->Read(ci))
    {
        LOG_WARNING << "Unable to  read concrete item " << ci.uuid << std::endl;
        return;
    }
    ci.deleted = Utils::Tick();
    if (client->Update(ci))
    {
        client->Invalidate(ci);
    }
    else
        LOG_WARNING << "Error deleting concrete item " << ci.uuid << std::endl;
}

void ItemFactory::DeleteItem(Item* item)
{
    if (!item)
        return;

    for (size_t i = 0; i < static_cast<size_t>(ItemUpgrade::__Count); ++i)
    {
        if (auto* upgrade = item->GetUpgrade(static_cast<ItemUpgrade>(i)))
        {
            DeleteConcrete(upgrade->concreteItem_.uuid);
            item->RemoveUpgrade(ItemUpgrade::Pefix);
        }
    }
    DeleteConcrete(item->concreteItem_.uuid);
}

void ItemFactory::InternalLoadDropChances(const std::string mapUuid, bool force)
{
    if (!force)
    {
        auto it = dropChances_.find(mapUuid);
        if (it != dropChances_.end())
            // Already loaded
            return;
    }

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
        if (i.canDrop)
            selector->Add(i.itemUuid, static_cast<float>(i.chance) / 1000.0f);
    }
    selector->Update();

    std::scoped_lock lock(lock_);
    dropChances_.emplace(mapUuid, std::move(selector));
}

void ItemFactory::LoadDropChances(const std::string mapUuid)
{
    InternalLoadDropChances(mapUuid, false);
}

void ItemFactory::ReloadDropChances()
{
    ea::vector<std::string> mapsToLoad;
    mapsToLoad.reserve(dropChances_.size());
    for (const auto& c : dropChances_)
        mapsToLoad.push_back(c.first);

    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    for (const auto& mapUuid : mapsToLoad)
    {
        AB::Entities::ItemChanceList il;
        il.uuid = mapUuid;
        client->Invalidate(il);
        InternalLoadDropChances(mapUuid, true);
    }
}

void ItemFactory::DeleteMap(const std::string& uuid)
{
    auto it = dropChances_.find(uuid);
    if (it != dropChances_.end())
        dropChances_.erase(it);
}

uint32_t ItemFactory::CreateDropItem(const std::string& instanceUuid, const std::string& mapUuid,
    uint32_t level, Player* target)
{
    AB_PROFILE;
    auto it = dropChances_.find(mapUuid);
    if (it == dropChances_.end())
        // Maybe outpost -> no drops
        return 0;
    if (!(*it).second || (*it).second->Count() == 0)
        // No drops on this map :(
        return 0;

    auto* rng = GetSubsystem<Crypto::Random>();
    const float rnd1 = rng->GetFloat();
    const float rnd2 = rng->GetFloat();
    const std::string& itemUuid = (*it).second->Get(rnd1, rnd2);
    if (uuids::uuid(itemUuid).nil())
        // There is a chance that nothing drops
        return 0;

    return CreateItem({ itemUuid, instanceUuid, mapUuid, level, false, target->account_.uuid, target->data_.uuid,
        0, 0, AB::Entities::StoragePlace::Scene });
}

bool ItemFactory::MoveToMerchant(Item* item, uint32_t count)
{
    ASSERT(count <= item->concreteItem_.count);
    auto* cache = GetSubsystem<ItemsCache>();
    if (!item->IsResellable() || AB::Entities::IsItemCustomized(item->concreteItem_.flags))
    {
        // Only resellable items are kept by the Merchant, all others get deleted.
        DeleteItem(item);
        cache->Remove(item->id_);
        return true;
    }

    auto* dc = GetSubsystem<IO::DataClient>();

    if (!item->IsStackable())
    {
        // If not stackable we just move the existing item to the merchant
        item->concreteItem_.storagePlace = AB::Entities::StoragePlace::Merchant;
        item->concreteItem_.storagePos = 0;
        item->concreteItem_.playerUuid = Utils::Uuid::EMPTY_UUID;
        item->concreteItem_.accountUuid = Utils::Uuid::EMPTY_UUID;
        item->concreteItem_.sold = Utils::Tick();
        dc->Update(item->concreteItem_);
        dc->Invalidate(item->concreteItem_);
        // Merchant got new items.
        AB::Entities::MerchantItemList ml;
        dc->Invalidate(ml);
        // The ItemsCache is only used for player owned items, i.e. items in their inventory or chest.
        cache->Remove(item->id_);
        return true;
    }

    // The item is stackable -> Merge it if the merchant has already an item of this type
    AB::Entities::MerchantItem mi;
    mi.uuid = item->concreteItem_.itemUuid;
    if (dc->Read(mi))
    {
        AB::Entities::ConcreteItem ci;
        ci.uuid = mi.concreteUuid;
        IO::EntityLocker locker(*dc, ci);
        if (!locker.Lock())
        {
            LOG_ERROR << "Unable to lock concrete item " << ci.uuid << std::endl;
            return false;
        }
        // At this point the concrete item must exist
        if (!dc->Read(ci))
        {
            LOG_ERROR << "Unable to read concrete item " << ci.uuid << std::endl;
            return false;
        }
        // There is not stack size for merchants, they have a huuuuge bag
        ASSERT(!Utils::WouldExceed(ci.count, count, std::numeric_limits<decltype(ci.count)>::max()));
        ci.count += count;
        ci.sold = Utils::Tick();
        dc->Update(ci);
        if (item->concreteItem_.count == count)
        {
            // The player gave us all so delete the player item
            DeleteItem(item);
            cache->Remove(item->id_);
        }
        else
            item->concreteItem_.count -= count;
        dc->Invalidate(ci);
        dc->Invalidate(mi);
        return true;
    }

    // First time we get an item of this type
    if (item->concreteItem_.count == count)
    {
        // If the player gave us all we can just move the whole stack
        item->concreteItem_.storagePlace = AB::Entities::StoragePlace::Merchant;
        item->concreteItem_.storagePos = 0;
        item->concreteItem_.playerUuid = Utils::Uuid::EMPTY_UUID;
        item->concreteItem_.accountUuid = Utils::Uuid::EMPTY_UUID;
        item->concreteItem_.sold = Utils::Tick();
        dc->Update(item->concreteItem_);
        dc->Invalidate(item->concreteItem_);
        // Merchant got new items.
        AB::Entities::MerchantItemList ml;
        dc->Invalidate(ml);
        // The ItemsCache is only used for player owned items, i.e. items in their inventory or chest.
        cache->Remove(item->id_);
        return true;
    }

    // We don't have this item yet, and the player didn't give us all, so we need to create a new concrete item
    AB::Entities::ConcreteItem ci = item->concreteItem_;
    ci.uuid = Utils::Uuid::New();
    ci.count = count;
    ci.storagePlace = AB::Entities::StoragePlace::Merchant;
    ci.storagePos = 0;
    ci.playerUuid = Utils::Uuid::EMPTY_UUID;
    ci.accountUuid = Utils::Uuid::EMPTY_UUID;
    ci.sold = Utils::Tick();
    if (!dc->Create(ci))
    {
        // This should really not happen
        LOG_ERROR << "Error creating concrete item with uuid " << ci.uuid << std::endl;
        return false;
    }
    dc->Invalidate(ci);
    AB::Entities::MerchantItemList ml;
    dc->Invalidate(ml);

    item->concreteItem_.count -= count;
    dc->Update(item->concreteItem_);
    return true;
}

std::string ItemFactory::GetMaxItemStats(const std::string& itemUuid, uint32_t level)
{
    const ea::pair<size_t, uint32_t> key = { sa::StringHashRt(itemUuid.c_str()), level };
    const auto it = maxItemStats_.find(key);
    if (it != maxItemStats_.end())
        return (*it).second;

    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item item;
    item.uuid = itemUuid;
    if (!client->Read(item))
        return "";
    ea::unique_ptr<Item> pItem = ea::make_unique<Item>(item);
    if (!pItem->LoadScript(item.script))
        return "";
    AB::Entities::ConcreteItem ci;
    if (!pItem->GenerateConcrete(ci, level, true))
        return "";
    maxItemStats_.emplace(key, pItem->concreteItem_.itemStats);
    return pItem->concreteItem_.itemStats;
}

std::string ItemFactory::GetMaxItemStatsWithAttribute(const std::string& itemUuid, uint32_t level, Attribute attrib, int attribRank)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Item item;
    item.uuid = itemUuid;
    if (!client->Read(item))
        return "";
    ea::unique_ptr<Item> pItem = ea::make_unique<Item>(item);
    if (!pItem->LoadScript(item.script))
        return "";
    AB::Entities::ConcreteItem ci;
    if (!pItem->GenerateConcrete(ci, level, true))
        return "";
    pItem->stats_.SetValue(ItemStatIndex::Attribute, static_cast<int>(attrib));
    if (attribRank > -1)
        pItem->stats_.SetValue(ItemStatIndex::AttributeValue, attribRank);
    return pItem->GetEncodedStats();
}

uint32_t ItemFactory::GetItemIndexFromUuid(const std::string& uuid)
{
    AB::Entities::Item item;
    item.uuid = uuid;
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(item))
        return 0;
    return item.index;
}

}
