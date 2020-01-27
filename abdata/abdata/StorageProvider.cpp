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

#include "stdafx.h"
#include "StorageProvider.h"
#include "Database.h"
#include <sstream>
#include "Scheduler.h"
#include "Dispatcher.h"
#include "DBAll.h"
#include "StringUtils.h"
#include "Profiler.h"
#include <AB/Entities/Party.h>
#include "Subsystems.h"
#include "ThreadPool.h"
#include "Database.h"

static constexpr size_t KEY_ACCOUNTS_HASH = sa::StringHash(AB::Entities::Account::KEY());
static constexpr size_t KEY_CHARACTERS_HASH = sa::StringHash(AB::Entities::Character::KEY());
static constexpr size_t KEY_GAMES_HASH = sa::StringHash(AB::Entities::Game::KEY());
static constexpr size_t KEY_GAMELIST_HASH = sa::StringHash(AB::Entities::GameList::KEY());
static constexpr size_t KEY_IPBANS_HASH = sa::StringHash(AB::Entities::IpBan::KEY());
static constexpr size_t KEY_ACCOUNTBANS_HASH = sa::StringHash(AB::Entities::AccountBan::KEY());
static constexpr size_t KEY_BANS_HASH = sa::StringHash(AB::Entities::Ban::KEY());
static constexpr size_t KEY_FRIENDLIST_HASH = sa::StringHash(AB::Entities::FriendList::KEY());
static constexpr size_t KEY_ACCOUNTKEYS_HASH = sa::StringHash(AB::Entities::AccountKey::KEY());
static constexpr size_t KEY_ACCOUNTKEYACCOUNTS_HASH = sa::StringHash(AB::Entities::AccountKeyAccounts::KEY());
static constexpr size_t KEY_MAIL_HASH = sa::StringHash(AB::Entities::Mail::KEY());
static constexpr size_t KEY_MAILLIST_HASH = sa::StringHash(AB::Entities::MailList::KEY());
static constexpr size_t KEY_PROFESSIONS_HASH = sa::StringHash(AB::Entities::Profession::KEY());
static constexpr size_t KEY_SKILLS_HASH = sa::StringHash(AB::Entities::Skill::KEY());
static constexpr size_t KEY_EFFECTS_HASH = sa::StringHash(AB::Entities::Effect::KEY());
static constexpr size_t KEY_ATTRIBUTES_HASH = sa::StringHash(AB::Entities::Attribute::KEY());
static constexpr size_t KEY_SKILLLIST_HASH = sa::StringHash(AB::Entities::SkillList::KEY());
static constexpr size_t KEY_EFFECTLIST_HASH = sa::StringHash(AB::Entities::EffectList::KEY());
static constexpr size_t KEY_PROFESSIONLIST_HASH = sa::StringHash(AB::Entities::ProfessionList::KEY());
static constexpr size_t KEY_VERSIONS_HASH = sa::StringHash(AB::Entities::Version::KEY());
static constexpr size_t KEY_ATTRIBUTELIST_HASH = sa::StringHash(AB::Entities::AttributeList::KEY());
static constexpr size_t KEY_SERVICE_HASH = sa::StringHash(AB::Entities::Service::KEY());
static constexpr size_t KEY_SERVICELIST_HASH = sa::StringHash(AB::Entities::ServiceList::KEY());
static constexpr size_t KEY_GUILD_HASH = sa::StringHash(AB::Entities::Guild::KEY());
static constexpr size_t KEY_GUILDMEMBERS_HASH = sa::StringHash(AB::Entities::GuildMembers::KEY());
static constexpr size_t KEY_RESERVEDNAME_HASH = sa::StringHash(AB::Entities::ReservedName::KEY());
static constexpr size_t KEY_GAMEINSTANCES_HASH = sa::StringHash(AB::Entities::GameInstance::KEY());
static constexpr size_t KEY_ITEMS_HASH = sa::StringHash(AB::Entities::Item::KEY());
static constexpr size_t KEY_ITEMLIST_HASH = sa::StringHash(AB::Entities::ItemList::KEY());
static constexpr size_t KEY_VERSIONLIST_HASH = sa::StringHash(AB::Entities::VersionList::KEY());
static constexpr size_t KEY_ACCOUNTLIST_HASH = sa::StringHash(AB::Entities::AccountList::KEY());
static constexpr size_t KEY_CHARACTERLIST_HASH = sa::StringHash(AB::Entities::CharacterList::KEY());
static constexpr size_t KEY_ACCOUNTKEYLIST_HASH = sa::StringHash(AB::Entities::AccountKeyList::KEY());
static constexpr size_t KEY_MUSIC_HASH = sa::StringHash(AB::Entities::Music::KEY());
static constexpr size_t KEY_MUSICLIST_HASH = sa::StringHash(AB::Entities::MusicList::KEY());
static constexpr size_t KEY_PARTIES_HASH = sa::StringHash(AB::Entities::Party::KEY());
static constexpr size_t KEY_CONCRETEITEMS_HASH = sa::StringHash(AB::Entities::ConcreteItem::KEY());
static constexpr size_t KEY_ACCOUNTITEMLIST_HASH = sa::StringHash(AB::Entities::AccountItemList::KEY());
static constexpr size_t KEY_CHESTITEMLIST_HASH = sa::StringHash(AB::Entities::ChestItems::KEY());
static constexpr size_t KEY_PLAYERITEMLIST_HASH = sa::StringHash(AB::Entities::PlayerItemList::KEY());
static constexpr size_t KEY_INVENTORYITEMLIST_HASH = sa::StringHash(AB::Entities::InventoryItems::KEY());
static constexpr size_t KEY_EQUIPPEDITEMLIST_HASH = sa::StringHash(AB::Entities::EquippedItems::KEY());
static constexpr size_t KEY_ITEMCHANCELIST_HASH = sa::StringHash(AB::Entities::ItemChanceList::KEY());
static constexpr size_t KEY_TYPEDITEMLIST_HASH = sa::StringHash(AB::Entities::TypedItemList::KEY());
static constexpr size_t KEY_INSIGNIAITEMLIST_HASH = sa::StringHash(AB::Entities::TypedItemsInsignia::KEY());
static constexpr size_t KEY_RUNEITEMLIST_HASH = sa::StringHash(AB::Entities::TypedItemsRunes::KEY());
static constexpr size_t KEY_WEAPONPREFIXITEMLIST_HASH = sa::StringHash(AB::Entities::TypedItemsWeaponPrefix::KEY());
static constexpr size_t KEY_WEAPONSUFFIXITEMLIST_HASH = sa::StringHash(AB::Entities::TypedItemsWeaponSuffix::KEY());
static constexpr size_t KEY_WEAPONINSCRIPTIONITEMLIST_HASH = sa::StringHash(AB::Entities::TypedItemsWeaponInscription::KEY());

StorageProvider::StorageProvider(size_t maxSize, bool readonly) :
    flushInterval_(FLUSH_CACHE_MS),
    cleanInterval_(CLEAN_CACHE_MS),
    readonly_(readonly),
    running_(true),
    maxSize_(maxSize),
    currentSize_(0),
    cache_()
{
    InitEnitityClasses();
    auto sched = GetSubsystem<Asynch::Scheduler>();
    sched->Add(
        Asynch::CreateScheduledTask(FLUSH_CACHE_MS, std::bind(&StorageProvider::FlushCacheTask, this))
    );
    sched->Add(
        Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&StorageProvider::CleanTask, this))
    );
}

void StorageProvider::InitEnitityClasses()
{
    AddEntityClass<DB::DBAccount, AB::Entities::Account>();
    AddEntityClass<DB::DBCharacter, AB::Entities::Character>();
    AddEntityClass<DB::DBGame, AB::Entities::Game>();
    AddEntityClass<DB::DBGameList, AB::Entities::GameList>();
    AddEntityClass<DB::DBIpBan, AB::Entities::IpBan>();
    AddEntityClass<DB::DBAccountBan, AB::Entities::AccountBan>();
    AddEntityClass<DB::DBBan, AB::Entities::Ban>();
    AddEntityClass<DB::DBFriendList, AB::Entities::FriendList>();
    AddEntityClass<DB::DBAccountKey, AB::Entities::AccountKey>();
    AddEntityClass<DB::DBAccountKeyAccounts, AB::Entities::AccountKeyAccounts>();
    AddEntityClass<DB::DBMail, AB::Entities::Mail>();
    AddEntityClass<DB::DBMailList, AB::Entities::MailList>();
    AddEntityClass<DB::DBProfession, AB::Entities::Profession>();
    AddEntityClass<DB::DBSkill, AB::Entities::Skill>();
    AddEntityClass<DB::DBEffect, AB::Entities::Effect>();
    AddEntityClass<DB::DBAttribute, AB::Entities::Attribute>();
    AddEntityClass<DB::DBSkillList, AB::Entities::SkillList>();
    AddEntityClass<DB::DBEffectList, AB::Entities::EffectList>();
    AddEntityClass<DB::DBProfessionList, AB::Entities::ProfessionList>();
    AddEntityClass<DB::DBVersion, AB::Entities::Version>();
    AddEntityClass<DB::DBAttributeList, AB::Entities::AttributeList>();
    AddEntityClass<DB::DBAccountList, AB::Entities::AccountList>();
    AddEntityClass<DB::DBCharacterList, AB::Entities::CharacterList>();
    AddEntityClass<DB::DBService, AB::Entities::Service>();
    AddEntityClass<DB::DBServicelList, AB::Entities::ServiceList>();
    AddEntityClass<DB::DBGuild, AB::Entities::Guild>();
    AddEntityClass<DB::DBGuildMembers, AB::Entities::GuildMembers>();
    AddEntityClass<DB::DBReservedName, AB::Entities::ReservedName>();
    AddEntityClass<DB::DBItem, AB::Entities::Item>();
    AddEntityClass<DB::DBItemList, AB::Entities::ItemList>();
    AddEntityClass<DB::DBVersionList, AB::Entities::VersionList>();
    AddEntityClass<DB::DBAccountKeyList, AB::Entities::AccountKeyList>();
    AddEntityClass<DB::DBMusic, AB::Entities::Music>();
    AddEntityClass<DB::DBMusicList, AB::Entities::MusicList>();
    AddEntityClass<DB::DBConcreteItem, AB::Entities::ConcreteItem>();
    AddEntityClass<DB::DBAccountItemList, AB::Entities::ChestItems>();
    AddEntityClass<DB::DBPlayerItemList, AB::Entities::InventoryItems>();
    AddEntityClass<DB::DBPlayerItemList, AB::Entities::EquippedItems>();
    AddEntityClass<DB::DBItemChanceList, AB::Entities::ItemChanceList>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsInsignia>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsRunes>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsWeaponPrefix>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsWeaponSuffix>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsWeaponInscription>();
    AddEntityClass<DB::DBInstance, AB::Entities::GameInstance>();
    AddEntityClass<DB::DBFriendedMe, AB::Entities::FriendedMe>();
    AddEntityClass<DB::DBQuest, AB::Entities::Quest>();
    AddEntityClass<DB::DBQuestList, AB::Entities::QuestList>();
    AddEntityClass<DB::DBPlayerQuest, AB::Entities::PlayerQuest>();
    AddEntityClass<DB::DBPlayerQuestList, AB::Entities::PlayerQuestList>();
    AddEntityClass<DB::DBPlayerQuestListRewarded, AB::Entities::PlayerQuestListRewarded>();
}

bool StorageProvider::Create(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    auto _data = cache_.find(key);

    if (_data != cache_.end())
    {
        // If there is a deleted record we must delete it from DB now or we may get
        // a constraint violation.
        if ((*_data).second.first.deleted)
            FlushData(key);
        else
            // Already exists
            return false;
    }

    std::string table;
    uuids::uuid _id;
    if (!key.decode(table, _id))
    {
        LOG_ERROR << "Unable to decode key, key size " << key.data_.size() << std::endl;
        return false;
    }
    if (_id.nil())
    {
        LOG_ERROR << "UUID is nil, table is " << table << std::endl;
        return false;
    }

    CacheData(table, _id, data, false, false);

    // Unfortunately we must flush the data for create operations. Or we find a way
    // to check constraints, unique columns etc.
    if (!FlushData(key))
    {
        // Failed delete from cache
        RemoveData(key);
        return false;
    }
    return true;
}

bool StorageProvider::Update(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return false;
    }

    if (id.nil())
        // Does not exist
        return false;

    auto _data = cache_.find(key);
    // The only possibility to update a not yet created entity is when its in cache and not flushed.
    bool isCreated = true;
    if (_data != cache_.end())
        isCreated = (*_data).second.first.created;

    // The client sets the data so this is not stored in DB
    CacheData(table, id, data, true, isCreated);
    return true;
}

void StorageProvider::CacheData(const std::string& table, const uuids::uuid& id,
    std::shared_ptr<std::vector<uint8_t>> data, bool modified, bool created)
{
    size_t sizeNeeded = data->size();
    if (!EnoughSpace(sizeNeeded))
    {
        // Create space later
        GetSubsystem<Asynch::Dispatcher>()->Add(
            Asynch::CreateTask(std::bind(&StorageProvider::CreateSpace, this, sizeNeeded))
        );
    }

    const IO::DataKey key(table, id);

    // we check if its already in cache
    if (cache_.find(key) == cache_.end())
    {
        evictor_.AddKey(key);
        currentSize_ += data->size();
    }
    else
    {
        evictor_.RefreshKey(key);
        currentSize_ = (currentSize_ - cache_[key].second->size()) + data->size();
    }

    cache_[key] = { { created, modified, false }, data };

    // Special case for player names
    size_t tableHash = sa::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        AB::Entities::Character ch;
        if (GetEntity(*data, ch))
            playerNames_[Utils::Utf8ToLower(ch.name)] = key;
    }
}

bool StorageProvider::Read(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data)
{
//    AB_PROFILE;

    auto _data = cache_.find(key);
    if (_data != cache_.end())
    {
        if ((*_data).second.first.deleted)
            // Don't return deleted items that are in cache
            return false;
        data->assign((*_data).second.second->begin(), (*_data).second.second->end());
        return true;
    }

    std::string table;
    uuids::uuid _id;
    if (!key.decode(table, _id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return false;
    }

    // Maybe in player names cache
    // Special case for player names
    size_t tableHash = sa::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        AB::Entities::Character ch;
        if (GetEntity(*data, ch) && !ch.name.empty())
        {
            auto playerIt = playerNames_.find(Utils::Utf8ToLower(ch.name));
            if (playerIt != playerNames_.end())
            {
                const auto& playerKey = (*playerIt).second;
                _data = cache_.find(playerKey);
                if (_data != cache_.end())
                {
                    if ((*_data).second.first.deleted)
                        // Don't return deleted items that are in cache
                        return false;
                    data->assign((*_data).second.second->begin(), (*_data).second.second->end());
                    return true;
                }
            }
        }
    }

    // Really not in cache
    if (!LoadData(key, data))
        return false;

    if (_id.nil())
        // If no UUID given in key (e.g. when reading by name) cache with the proper key
        _id = GetUuid(*data);
    const IO::DataKey newKey(table, _id);
    auto _newdata = cache_.find(newKey);
    if (_newdata == cache_.end())
    {
        CacheData(table, _id, data, false, true);
    }
    else
    {
        // Was already cached
        if ((*_newdata).second.first.deleted)
            // Don't return deleted items that are in cache
            return false;
        // Return the cached object, it may have changed
        data->assign((*_newdata).second.second->begin(), (*_newdata).second.second->end());
    }
    return true;

}

bool StorageProvider::Delete(const IO::DataKey& key)
{
    // You can only delete what you've loaded before
    auto data = cache_.find(key);
    if (data == cache_.end())
    {
        return false;
    }
    (*data).second.first.deleted = true;
    return true;
}

bool StorageProvider::Invalidate(const IO::DataKey& key)
{
    if (!FlushData(key))
    {
        LOG_ERROR << "Error flushing " << key.format() << std::endl;
        return false;
    }
    return RemoveData(key);
}

void StorageProvider::PreloadTask(IO::DataKey key)
{
    // Dispatcher thread

    auto _data = cache_.find(key);
    if (_data == cache_.end())
        return;

    std::string table;
    uuids::uuid _id;
    if (!key.decode(table, _id))
    {
        LOG_ERROR << "Unable to decode key " << key.format() << std::endl;
        return;
    }
    std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>(0);
    if (!LoadData(key, data))
        return;

    if (_id.nil())
        // If no UUID given in key (e.g. when reading by name) cache with the proper key
        _id = GetUuid(*data);
    IO::DataKey newKey(table, _id);

    auto _newdata = cache_.find(newKey);
    if (_newdata == cache_.end())
    {
        CacheData(table, _id, data, false, true);
    }
}

bool StorageProvider::Preload(const IO::DataKey& key)
{
    auto _data = cache_.find(key);
    if (_data == cache_.end())
        return true;

    // Load later
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(&StorageProvider::PreloadTask, this, key))
    );
    return true;
}

bool StorageProvider::Exists(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    auto _data = cache_.find(key);

    if (_data != cache_.end())
        return !(*_data).second.first.deleted;

    return ExistsData(key, *data);
}

bool StorageProvider::Clear(const IO::DataKey&)
{
    std::vector<IO::DataKey> toDelete;

    for (const auto& ci : cache_)
    {
        const IO::DataKey& key = ci.first;
        std::string table;
        uuids::uuid id;
        if (!key.decode(table, id))
            continue;
        size_t tableHash = sa::StringHashRt(table.data());
        if (tableHash == KEY_GAMEINSTANCES_HASH || tableHash == KEY_SERVICE_HASH || tableHash == KEY_PARTIES_HASH)
            // Can not delete these
            continue;

        if (FlushData(key))
        {
            currentSize_ -= ci.second.second->size();
            toDelete.push_back(key);
        }
    }
    for (const auto& k : toDelete)
    {
        cache_.erase(k);
        evictor_.DeleteKey(k);
    }
    playerNames_.clear();
    LOG_INFO << "Cleared cache, removed " << toDelete.size() << " items" << std::endl;
    return true;
}

void StorageProvider::Shutdown()
{
    // The only thing that not called from the dispatcher thread, so lock it.
    std::lock_guard<std::mutex> lock(lock_);
    running_ = false;
    for (const auto& c : cache_)
        FlushData(c.first);

    DB::DBAccount::LogoutAll();
}

void StorageProvider::CleanCache()
{
    // Delete deleted records from DB and remove them from cache.
    if (cache_.size() == 0)
        return;
    size_t oldSize = currentSize_;
    int removed = 0;
    auto i = cache_.begin();
    while ((i = std::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        return current.second.first.deleted;
    })) != cache_.end())
    {
        bool ok = true;
        const IO::DataKey& key = (*i).first;
        if ((*i).second.first.created)
        {
            // If it's in DB (created == true) update changed data in DB
            ok = FlushData(key);
        }
        if (ok)
        {
            // Remove from players cache
            RemovePlayerFromCache(key);
            currentSize_ -= (*i).second.second->size();
            evictor_.DeleteKey(key);
            cache_.erase(i++);
            ++removed;
        }
        else
        {
            // Error, break for now and try  the next time.
            // In case of lost connection it would try forever.
            break;
        }
    }

    if (removed > 0)
    {
        LOG_INFO << "Cleaned cache old size " << Utils::ConvertSize(oldSize) <<
            " current size " << Utils::ConvertSize(currentSize_) <<
            " removed " << removed << " record(s)" << std::endl;
    }
}

void StorageProvider::CleanTask()
{
    AB_PROFILE;
    CleanCache();
    DB::DBGuildMembers::DeleteExpired(this);
    DB::DBReservedName::DeleteExpired(this);
//    DB::DBConcreteItem::Clean(this);
    if (running_)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(cleanInterval_, std::bind(&StorageProvider::CleanTask, this))
        );
    }
}

void StorageProvider::FlushCache()
{
    if (cache_.size() == 0)
        return;
    AB_PROFILE;
    int written = 0;
    auto i = cache_.begin();
    auto* tp = GetSubsystem<Asynch::ThreadPool>();
    while ((i = std::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        // Don't return deleted, these are flushed in CleanCache()
        return (current.second.first.modified || !current.second.first.created) &&
            !current.second.first.deleted;
    })) != cache_.end())
    {
        ++written;
        const IO::DataKey& key = (*i).first;
        auto res = tp->EnqueueWithResult(&StorageProvider::FlushData, this, key);
        bool bRes = false;
        {
            std::lock_guard<std::mutex> lock(lock_);
            bRes = res.get();
        }
        if (!bRes)
        {
            LOG_WARNING << "Error flushing " << key.format() << std::endl;
            // Error, break for now and try  the next time.
            // In case of lost connection it would try forever.
            break;
        }
    }
    if (written > 0)
    {
        LOG_INFO << "Flushed cache wrote " << written << " record(s)" << std::endl;
    }
}

void StorageProvider::FlushCacheTask()
{
    DB::Database* db = GetSubsystem<DB::Database>();
    db->CheckConnection();
    FlushCache();
    if (running_)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(flushInterval_, std::bind(&StorageProvider::FlushCacheTask, this))
        );
    }
}

uuids::uuid StorageProvider::GetUuid(std::vector<uint8_t>& data)
{
    // Get UUID from raw data. UUID is serialized first as string
    const std::string suuid(data.begin() + 1,
        data.begin() + AB::Entities::Limits::MAX_UUID + 1);
    return uuids::uuid(suuid);
}

bool StorageProvider::EnoughSpace(size_t size)
{
    return (currentSize_ + size) <= maxSize_;
}

void StorageProvider::CreateSpace(size_t size)
{
    // Create more than required space
    const size_t sizeNeeded = size * 2;
    while ((currentSize_ + sizeNeeded) > maxSize_)
    {
        const IO::DataKey key = evictor_.NextEviction();
        if (FlushData(key))
            RemoveData(key);
        else
            break;
    }
}

bool StorageProvider::RemoveData(const IO::DataKey& key)
{
    auto data = cache_.find(key);
    if (data != cache_.end())
    {
        RemovePlayerFromCache(key);

        currentSize_ -= (*data).second.second->size();
        cache_.erase(key);
        evictor_.DeleteKey(key);

        return true;
    }
    return false;
}

bool StorageProvider::LoadData(const IO::DataKey& key,
    std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
    {
        LOG_ERROR << "Unable to decode key " << key.format() << std::endl;
        return false;
    }

    size_t tableHash = sa::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTITEMLIST_HASH:
        assert(false);
    case KEY_PLAYERITEMLIST_HASH:
        // Use one bellow
        assert(false);
    case KEY_TYPEDITEMLIST_HASH:
        assert(false);
    case KEY_PARTIES_HASH:
        // Not written to DB
        return false;
    default:
    {
        if (loadCallables_.Exists(tableHash))
            return loadCallables_.Call(tableHash, id, *data);
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }
    }

    return false;
}

bool StorageProvider::FlushData(const IO::DataKey& key)
{
    if (readonly_)
    {
        LOG_WARNING << "READONLY: Nothing is written to the database" << std::endl;
        return true;
    }

    auto data = cache_.find(key);
    if (data == cache_.end())
        // Not in cache so no need to flush anything
        return true;
    // No need to save to DB when not modified
    if (!(*data).second.first.modified && !(*data).second.first.deleted && (*data).second.first.created)
        return true;

    CacheItem& item = (*data).second;

    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
    {
        LOG_ERROR << "Unable to decode key " << key.format() << std::endl;
        return false;
    }

    size_t tableHash = sa::StringHashRt(table.data());
    bool succ = false;

    switch (tableHash)
    {
    case KEY_ACCOUNTITEMLIST_HASH:
        assert(false);
    case KEY_PLAYERITEMLIST_HASH:
        assert(false);
    case KEY_TYPEDITEMLIST_HASH:
        assert(false);
    case KEY_SERVICELIST_HASH:
    case KEY_INVENTORYITEMLIST_HASH:
    case KEY_EQUIPPEDITEMLIST_HASH:
    case KEY_PARTIES_HASH:
        // Not written to DB
        // Mark not modified and created or it will infinitely try to flush it
        (*data).second.first.created = true;
        (*data).second.first.modified = false;
        succ = true;
        break;
    default:
    {
        if (flushCallables_.Exists(tableHash))
            succ = flushCallables_.Call(tableHash, item);
        else
        {
            LOG_ERROR << "Unknown table " << table << std::endl;
            return false;
        }
    }
    }

    if (!succ)
        LOG_ERROR << "Unable to write data" << std::endl;
    return succ;
}

bool StorageProvider::ExistsData(const IO::DataKey& key, std::vector<uint8_t>& data)
{
    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
        return false;

    size_t tableHash = sa::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTITEMLIST_HASH:
        assert(false);
    case KEY_PLAYERITEMLIST_HASH:
        assert(false);
    case KEY_TYPEDITEMLIST_HASH:
        assert(false);
    case KEY_PARTIES_HASH:
        // Not written to DB. If we are here its not in cache so does not exist
        return false;
    default:
        if (exitsCallables_.Exists(tableHash))
            return exitsCallables_.Call(tableHash, data);
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }

    return false;
}

void StorageProvider::RemovePlayerFromCache(const IO::DataKey& key)
{
    std::string table;
    uuids::uuid playerUuid;
    if (!key.decode(table, playerUuid))
        return;

    size_t tableHash = sa::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        auto _data = cache_.find(key);
        if (_data == cache_.end())
            return;

        AB::Entities::Character ch;
        if (GetEntity(*(*_data).second.second, ch))
        {
            auto it = playerNames_.find(Utils::Utf8ToLower(ch.name));
            if (it != playerNames_.end())
                playerNames_.erase(it);
        }
    }
}
