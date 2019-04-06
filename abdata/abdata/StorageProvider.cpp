#include "stdafx.h"
#include "StorageProvider.h"
#include "Database.h"
#include <sstream>
#include "StringHash.h"
#include "StringHash.h"
#include "Scheduler.h"
#include "Dispatcher.h"
#include "DBAll.h"
#include "StringUtils.h"
#include "Profiler.h"
#include <AB/Entities/GameInstance.h>
#include <AB/Entities/Party.h>
#include "Subsystems.h"
#include "ThreadPool.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4307)
#endif
static constexpr size_t KEY_ACCOUNTS_HASH = Utils::StringHash(AB::Entities::Account::KEY());
static constexpr size_t KEY_CHARACTERS_HASH = Utils::StringHash(AB::Entities::Character::KEY());
static constexpr size_t KEY_GAMES_HASH = Utils::StringHash(AB::Entities::Game::KEY());
static constexpr size_t KEY_GAMELIST_HASH = Utils::StringHash(AB::Entities::GameList::KEY());
static constexpr size_t KEY_IPBANS_HASH = Utils::StringHash(AB::Entities::IpBan::KEY());
static constexpr size_t KEY_ACCOUNTBANS_HASH = Utils::StringHash(AB::Entities::AccountBan::KEY());
static constexpr size_t KEY_BANS_HASH = Utils::StringHash(AB::Entities::Ban::KEY());
static constexpr size_t KEY_FRIENDLIST_HASH = Utils::StringHash(AB::Entities::FriendList::KEY());
static constexpr size_t KEY_ACCOUNTKEYS_HASH = Utils::StringHash(AB::Entities::AccountKey::KEY());
static constexpr size_t KEY_ACCOUNTKEYACCOUNTS_HASH = Utils::StringHash(AB::Entities::AccountKeyAccounts::KEY());
static constexpr size_t KEY_MAIL_HASH = Utils::StringHash(AB::Entities::Mail::KEY());
static constexpr size_t KEY_MAILLIST_HASH = Utils::StringHash(AB::Entities::MailList::KEY());
static constexpr size_t KEY_PROFESSIONS_HASH = Utils::StringHash(AB::Entities::Profession::KEY());
static constexpr size_t KEY_SKILLS_HASH = Utils::StringHash(AB::Entities::Skill::KEY());
static constexpr size_t KEY_EFFECTS_HASH = Utils::StringHash(AB::Entities::Effect::KEY());
static constexpr size_t KEY_ATTRIBUTES_HASH = Utils::StringHash(AB::Entities::Attribute::KEY());
static constexpr size_t KEY_SKILLLIST_HASH = Utils::StringHash(AB::Entities::SkillList::KEY());
static constexpr size_t KEY_EFFECTLIST_HASH = Utils::StringHash(AB::Entities::EffectList::KEY());
static constexpr size_t KEY_PROFESSIONLIST_HASH = Utils::StringHash(AB::Entities::ProfessionList::KEY());
static constexpr size_t KEY_VERSIONS_HASH = Utils::StringHash(AB::Entities::Version::KEY());
static constexpr size_t KEY_ATTRIBUTELIST_HASH = Utils::StringHash(AB::Entities::AttributeList::KEY());
static constexpr size_t KEY_SERVICE_HASH = Utils::StringHash(AB::Entities::Service::KEY());
static constexpr size_t KEY_SERVICELIST_HASH = Utils::StringHash(AB::Entities::ServiceList::KEY());
static constexpr size_t KEY_GUILD_HASH = Utils::StringHash(AB::Entities::Guild::KEY());
static constexpr size_t KEY_GUILDMEMBERS_HASH = Utils::StringHash(AB::Entities::GuildMembers::KEY());
static constexpr size_t KEY_RESERVEDNAME_HASH = Utils::StringHash(AB::Entities::ReservedName::KEY());
static constexpr size_t KEY_GAMEINSTANCES_HASH = Utils::StringHash(AB::Entities::GameInstance::KEY());
static constexpr size_t KEY_ITEMS_HASH = Utils::StringHash(AB::Entities::Item::KEY());
static constexpr size_t KEY_ITEMLIST_HASH = Utils::StringHash(AB::Entities::ItemList::KEY());
static constexpr size_t KEY_VERSIONLIST_HASH = Utils::StringHash(AB::Entities::VersionList::KEY());
static constexpr size_t KEY_ACCOUNTLIST_HASH = Utils::StringHash(AB::Entities::AccountList::KEY());
static constexpr size_t KEY_CHARACTERLIST_HASH = Utils::StringHash(AB::Entities::CharacterList::KEY());
static constexpr size_t KEY_ACCOUNTKEYLIST_HASH = Utils::StringHash(AB::Entities::AccountKeyList::KEY());
static constexpr size_t KEY_MUSIC_HASH = Utils::StringHash(AB::Entities::Music::KEY());
static constexpr size_t KEY_MUSICLIST_HASH = Utils::StringHash(AB::Entities::MusicList::KEY());
static constexpr size_t KEY_PARTIES_HASH = Utils::StringHash(AB::Entities::Party::KEY());
static constexpr size_t KEY_CONCRETEITEMS_HASH = Utils::StringHash(AB::Entities::ConcreteItem::KEY());
static constexpr size_t KEY_ACCOUNTITEMLIST_HASH = Utils::StringHash(AB::Entities::AccountItemList::KEY());
static constexpr size_t KEY_CHESTITEMLIST_HASH = Utils::StringHash(AB::Entities::ChestItems::KEY());
static constexpr size_t KEY_PLAYERITEMLIST_HASH = Utils::StringHash(AB::Entities::PlayerItemList::KEY());
static constexpr size_t KEY_INVENTORYITEMLIST_HASH = Utils::StringHash(AB::Entities::InventoryItems::KEY());
static constexpr size_t KEY_EQUIPPEDITEMLIST_HASH = Utils::StringHash(AB::Entities::EquippedItems::KEY());
static constexpr size_t KEY_ITEMCHANCELIST_HASH = Utils::StringHash(AB::Entities::ItemChanceList::KEY());
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

StorageProvider::StorageProvider(size_t maxSize, bool readonly) :
    flushInterval_(FLUSH_CACHE_MS),
    cleanInterval_(CLEAN_CACHE_MS),
    readonly_(readonly),
    running_(true),
    maxSize_(maxSize),
    currentSize_(0),
    cache_()
{
    auto sched = GetSubsystem<Asynch::Scheduler>();
    sched->Add(
        Asynch::CreateScheduledTask(FLUSH_CACHE_MS, std::bind(&StorageProvider::FlushCacheTask, this))
    );
    sched->Add(
        Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&StorageProvider::CleanTask, this))
    );
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
    size_t tableHash = Utils::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        AB::Entities::Character ch;
        if (GetEntity(*data, ch))
        {
            // Avoid warning: https://developercommunity.visualstudio.com/content/problem/23811/stdtransform-with-toupper-fails-at-warning-level-4.html
            std::transform(ch.name.begin(), ch.name.end(), ch.name.begin(), [](char c) -> char { return static_cast<char>(std::tolower(static_cast<int>(c))); });
            playerNames_[ch.name] = key;
        }
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
    size_t tableHash = Utils::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        AB::Entities::Character ch;
        if (GetEntity(*data, ch) && !ch.name.empty())
        {
            std::transform(ch.name.begin(), ch.name.end(), ch.name.begin(), ::tolower);
            auto playerIt = playerNames_.find(ch.name);
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
        return false;
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
        size_t tableHash = Utils::StringHashRt(table.data());
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
    {
        FlushData(c.first);
    }
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
    auto tp = GetSubsystem<Asynch::ThreadPool>();
    while ((i = std::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        // Don't return deleted, these are flushed in CleanCache()
        return (current.second.first.modified || !current.second.first.created) &&
            !current.second.first.deleted;
    })) != cache_.end())
    {
        ++written;
        const IO::DataKey& key = (*i).first;
        // TOTO: Check this
        auto res = tp->EnqueueWithResult(&StorageProvider::FlushData, this, key);
        bool bRes = false;
        {
            std::lock_guard<std::mutex> lock(lock_);
            bRes = res.get();
        }
        if (!bRes)
//        if (!FlushData(key))
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

    size_t tableHash = Utils::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTS_HASH:
        return LoadFromDB<DB::DBAccount, AB::Entities::Account>(id, *data);
    case KEY_CHARACTERS_HASH:
        return LoadFromDB<DB::DBCharacter, AB::Entities::Character>(id, *data);
    case KEY_GAMES_HASH:
        return LoadFromDB<DB::DBGame, AB::Entities::Game>(id, *data);
    case KEY_GAMELIST_HASH:
        return LoadFromDB<DB::DBGameList, AB::Entities::GameList>(id, *data);
    case KEY_IPBANS_HASH:
        return LoadFromDB<DB::DBIpBan, AB::Entities::IpBan>(id, *data);
    case KEY_ACCOUNTBANS_HASH:
        return LoadFromDB<DB::DBAccountBan, AB::Entities::AccountBan>(id, *data);
    case KEY_BANS_HASH:
        return LoadFromDB<DB::DBBan, AB::Entities::Ban>(id, *data);
    case KEY_FRIENDLIST_HASH:
        return LoadFromDB<DB::DBFriendList, AB::Entities::FriendList>(id, *data);
    case KEY_ACCOUNTKEYS_HASH:
        return LoadFromDB<DB::DBAccountKey, AB::Entities::AccountKey>(id, *data);
    case KEY_ACCOUNTKEYACCOUNTS_HASH:
        return LoadFromDB<DB::DBAccountKeyAccounts, AB::Entities::AccountKeyAccounts>(id, *data);
    case KEY_MAIL_HASH:
        return LoadFromDB<DB::DBMail, AB::Entities::Mail>(id, *data);
    case KEY_MAILLIST_HASH:
        return LoadFromDB<DB::DBMailList, AB::Entities::MailList>(id, *data);
    case KEY_PROFESSIONS_HASH:
        return LoadFromDB<DB::DBProfession, AB::Entities::Profession>(id, *data);
    case KEY_SKILLS_HASH:
        return LoadFromDB<DB::DBSkill, AB::Entities::Skill>(id, *data);
    case KEY_EFFECTS_HASH:
        return LoadFromDB<DB::DBEffect, AB::Entities::Effect>(id, *data);
    case KEY_ATTRIBUTES_HASH:
        return LoadFromDB<DB::DBAttribute, AB::Entities::Attribute>(id, *data);
    case KEY_SKILLLIST_HASH:
        return LoadFromDB<DB::DBSkillList, AB::Entities::SkillList>(id, *data);
    case KEY_EFFECTLIST_HASH:
        return LoadFromDB<DB::DBEffectList, AB::Entities::EffectList>(id, *data);
    case KEY_PROFESSIONLIST_HASH:
        return LoadFromDB<DB::DBProfessionList, AB::Entities::ProfessionList>(id, *data);
    case KEY_VERSIONS_HASH:
        return LoadFromDB<DB::DBVersion, AB::Entities::Version>(id, *data);
    case KEY_ATTRIBUTELIST_HASH:
        return LoadFromDB<DB::DBAttributeList, AB::Entities::AttributeList>(id, *data);
    case KEY_ACCOUNTLIST_HASH:
        return LoadFromDB<DB::DBAccountList, AB::Entities::AccountList>(id, *data);
    case KEY_CHARACTERLIST_HASH:
        return LoadFromDB<DB::DBCharacterList, AB::Entities::CharacterList>(id, *data);
    case KEY_SERVICE_HASH:
        return LoadFromDB<DB::DBService, AB::Entities::Service>(id, *data);
    case KEY_SERVICELIST_HASH:
        return LoadFromDB<DB::DBServicelList, AB::Entities::ServiceList>(id, *data);
    case KEY_GUILD_HASH:
        return LoadFromDB<DB::DBGuild, AB::Entities::Guild>(id, *data);
    case KEY_GUILDMEMBERS_HASH:
        return LoadFromDB<DB::DBGuildMembers, AB::Entities::GuildMembers>(id, *data);
    case KEY_RESERVEDNAME_HASH:
        return LoadFromDB<DB::DBReservedName, AB::Entities::ReservedName>(id, *data);
    case KEY_ITEMS_HASH:
        return LoadFromDB<DB::DBItem, AB::Entities::Item>(id, *data);
    case KEY_ITEMLIST_HASH:
        return LoadFromDB<DB::DBItemList, AB::Entities::ItemList>(id, *data);
    case KEY_VERSIONLIST_HASH:
        return LoadFromDB<DB::DBVersionList, AB::Entities::VersionList>(id, *data);
    case KEY_ACCOUNTKEYLIST_HASH:
        return LoadFromDB<DB::DBAccountKeyList, AB::Entities::AccountKeyList>(id, *data);
    case KEY_MUSIC_HASH:
        return LoadFromDB<DB::DBMusic, AB::Entities::Music>(id, *data);
    case KEY_MUSICLIST_HASH:
        return LoadFromDB<DB::DBMusicList, AB::Entities::MusicList>(id, *data);
    case KEY_CONCRETEITEMS_HASH:
        return LoadFromDB<DB::DBConcreteItem, AB::Entities::ConcreteItem>(id, *data);
    case KEY_ACCOUNTITEMLIST_HASH:
    case KEY_CHESTITEMLIST_HASH:
        return LoadFromDB<DB::DBAccountItemList, AB::Entities::AccountItemList>(id, *data);
    case KEY_PLAYERITEMLIST_HASH:
    case KEY_INVENTORYITEMLIST_HASH:
    case KEY_EQUIPPEDITEMLIST_HASH:
        return LoadFromDB<DB::DBPlayerItemList, AB::Entities::PlayerItemList>(id, *data);
    case KEY_ITEMCHANCELIST_HASH:
        return LoadFromDB<DB::DBItemChanceList, AB::Entities::ItemChanceList>(id, *data);
    case KEY_GAMEINSTANCES_HASH:
    case KEY_PARTIES_HASH:
        // Not written to DB
        return false;
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
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
        return false;
    // No need to save to DB when not modified
    if (!(*data).second.first.modified && !(*data).second.first.deleted && (*data).second.first.created)
        return true;

    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
    {
        LOG_ERROR << "Unable to decode key " << key.format() << std::endl;
        return false;
    }

    size_t tableHash = Utils::StringHashRt(table.data());
    bool succ = false;

    switch (tableHash)
    {
    case KEY_ACCOUNTS_HASH:
        succ = FlushRecord<DB::DBAccount, AB::Entities::Account>(data);
        break;
    case KEY_CHARACTERS_HASH:
        succ = FlushRecord<DB::DBCharacter, AB::Entities::Character>(data);
        break;
    case KEY_GAMES_HASH:
        succ = FlushRecord<DB::DBGame, AB::Entities::Game>(data);
        break;
    case KEY_GAMELIST_HASH:
        succ = FlushRecord<DB::DBGameList, AB::Entities::GameList>(data);
        break;
    case KEY_IPBANS_HASH:
        succ = FlushRecord<DB::DBIpBan, AB::Entities::IpBan>(data);
        break;
    case KEY_ACCOUNTBANS_HASH:
        succ = FlushRecord<DB::DBAccountBan, AB::Entities::AccountBan>(data);
        break;
    case KEY_BANS_HASH:
        succ = FlushRecord<DB::DBBan, AB::Entities::Ban>(data);
        break;
    case KEY_FRIENDLIST_HASH:
        succ = FlushRecord<DB::DBFriendList, AB::Entities::FriendList>(data);
        break;
    case KEY_ACCOUNTKEYS_HASH:
        succ = FlushRecord<DB::DBAccountKey, AB::Entities::AccountKey>(data);
        break;
    case KEY_ACCOUNTKEYACCOUNTS_HASH:
        succ = FlushRecord<DB::DBAccountKeyAccounts, AB::Entities::AccountKeyAccounts>(data);
        break;
    case KEY_MAIL_HASH:
        succ = FlushRecord<DB::DBMail, AB::Entities::Mail>(data);
        break;
    case KEY_MAILLIST_HASH:
        succ = FlushRecord<DB::DBMailList, AB::Entities::MailList>(data);
        break;
    case KEY_PROFESSIONS_HASH:
        succ = FlushRecord<DB::DBProfession, AB::Entities::Profession>(data);
        break;
    case KEY_SKILLS_HASH:
        succ = FlushRecord<DB::DBSkill, AB::Entities::Skill>(data);
        break;
    case KEY_EFFECTS_HASH:
        succ = FlushRecord<DB::DBEffect, AB::Entities::Effect>(data);
        break;
    case KEY_ATTRIBUTES_HASH:
        succ = FlushRecord<DB::DBAttribute, AB::Entities::Attribute>(data);
        break;
    case KEY_SKILLLIST_HASH:
        succ = FlushRecord<DB::DBSkillList, AB::Entities::SkillList>(data);
        break;
    case KEY_EFFECTLIST_HASH:
        succ = FlushRecord<DB::DBEffectList, AB::Entities::EffectList>(data);
        break;
    case KEY_PROFESSIONLIST_HASH:
        succ = FlushRecord<DB::DBProfessionList, AB::Entities::ProfessionList>(data);
        break;
    case KEY_VERSIONS_HASH:
        succ = FlushRecord<DB::DBVersion, AB::Entities::Version>(data);
        break;
    case KEY_ATTRIBUTELIST_HASH:
        succ = FlushRecord<DB::DBAttributeList, AB::Entities::AttributeList>(data);
        break;
    case KEY_ACCOUNTLIST_HASH:
        succ = FlushRecord<DB::DBAccountList, AB::Entities::AccountList>(data);
        break;
    case KEY_CHARACTERLIST_HASH:
        succ = FlushRecord<DB::DBCharacterList, AB::Entities::CharacterList>(data);
        break;
    case KEY_SERVICE_HASH:
        succ = FlushRecord<DB::DBService, AB::Entities::Service>(data);
        break;
    case KEY_SERVICELIST_HASH:
        succ = FlushRecord<DB::DBServicelList, AB::Entities::ServiceList>(data);
        break;
    case KEY_GUILD_HASH:
        succ = FlushRecord<DB::DBGuild, AB::Entities::Guild>(data);
        break;
    case KEY_GUILDMEMBERS_HASH:
        succ = FlushRecord<DB::DBGuildMembers, AB::Entities::GuildMembers>(data);
        break;
    case KEY_RESERVEDNAME_HASH:
        succ = FlushRecord<DB::DBReservedName, AB::Entities::ReservedName>(data);
        break;
    case KEY_ITEMS_HASH:
        succ = FlushRecord<DB::DBItem, AB::Entities::Item>(data);
        break;
    case KEY_ITEMLIST_HASH:
        succ = FlushRecord<DB::DBItemList, AB::Entities::ItemList>(data);
        break;
    case KEY_VERSIONLIST_HASH:
        succ = FlushRecord<DB::DBVersionList, AB::Entities::VersionList>(data);
        break;
    case KEY_ACCOUNTKEYLIST_HASH:
        succ = FlushRecord<DB::DBAccountKeyList, AB::Entities::AccountKeyList>(data);
        break;
    case KEY_MUSIC_HASH:
        succ = FlushRecord<DB::DBMusic, AB::Entities::Music>(data);
        break;
    case KEY_MUSICLIST_HASH:
        succ = FlushRecord<DB::DBMusicList, AB::Entities::MusicList>(data);
        break;
    case KEY_CONCRETEITEMS_HASH:
        succ = FlushRecord<DB::DBConcreteItem, AB::Entities::ConcreteItem>(data);
        break;
    case KEY_ACCOUNTITEMLIST_HASH:
    case KEY_CHESTITEMLIST_HASH:
        succ = FlushRecord<DB::DBAccountItemList, AB::Entities::AccountItemList>(data);
        break;
    case KEY_PLAYERITEMLIST_HASH:
    case KEY_INVENTORYITEMLIST_HASH:
    case KEY_EQUIPPEDITEMLIST_HASH:
        succ = FlushRecord<DB::DBPlayerItemList, AB::Entities::PlayerItemList>(data);
        break;
    case KEY_ITEMCHANCELIST_HASH:
        succ = FlushRecord<DB::DBItemChanceList, AB::Entities::ItemChanceList>(data);
        break;
    case KEY_GAMEINSTANCES_HASH:
    case KEY_PARTIES_HASH:
        // Not written to DB
        // Mark not modified and created or it will infinitely try to flush it
        (*data).second.first.created = true;
        (*data).second.first.modified = false;
        succ = true;
        break;
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        return false;
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

    size_t tableHash = Utils::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTS_HASH:
        return ExistsInDB<DB::DBAccount, AB::Entities::Account>(data);
    case KEY_CHARACTERS_HASH:
        return ExistsInDB<DB::DBCharacter, AB::Entities::Character>(data);
    case KEY_GAMES_HASH:
        return ExistsInDB<DB::DBGame, AB::Entities::Game>(data);
    case KEY_GAMELIST_HASH:
        return ExistsInDB<DB::DBGameList, AB::Entities::GameList>(data);
    case KEY_IPBANS_HASH:
        return ExistsInDB<DB::DBIpBan, AB::Entities::IpBan>(data);
    case KEY_ACCOUNTBANS_HASH:
        return ExistsInDB<DB::DBAccountBan, AB::Entities::AccountBan>(data);
    case KEY_BANS_HASH:
        return ExistsInDB<DB::DBBan, AB::Entities::Ban>(data);
    case KEY_FRIENDLIST_HASH:
        return ExistsInDB<DB::DBFriendList, AB::Entities::FriendList>(data);
    case KEY_ACCOUNTKEYS_HASH:
        return ExistsInDB<DB::DBAccountKey, AB::Entities::AccountKey>(data);
    case KEY_ACCOUNTKEYACCOUNTS_HASH:
        return ExistsInDB<DB::DBAccountKeyAccounts, AB::Entities::AccountKeyAccounts>(data);
    case KEY_MAIL_HASH:
        return ExistsInDB<DB::DBMail, AB::Entities::Mail>(data);
    case KEY_MAILLIST_HASH:
        return ExistsInDB<DB::DBMailList, AB::Entities::MailList>(data);
    case KEY_PROFESSIONS_HASH:
        return ExistsInDB<DB::DBProfession, AB::Entities::Profession>(data);
    case KEY_SKILLS_HASH:
        return ExistsInDB<DB::DBSkill, AB::Entities::Skill>(data);
    case KEY_EFFECTS_HASH:
        return ExistsInDB<DB::DBEffect, AB::Entities::Effect>(data);
    case KEY_ATTRIBUTES_HASH:
        return ExistsInDB<DB::DBAttribute, AB::Entities::Attribute>(data);
    case KEY_SKILLLIST_HASH:
        return ExistsInDB<DB::DBSkillList, AB::Entities::SkillList>(data);
    case KEY_EFFECTLIST_HASH:
        return ExistsInDB<DB::DBEffectList, AB::Entities::EffectList>(data);
    case KEY_PROFESSIONLIST_HASH:
        return ExistsInDB<DB::DBProfessionList, AB::Entities::ProfessionList>(data);
    case KEY_VERSIONS_HASH:
        return ExistsInDB<DB::DBVersion, AB::Entities::Version>(data);
    case KEY_ATTRIBUTELIST_HASH:
        return ExistsInDB<DB::DBAttributeList, AB::Entities::AttributeList>(data);
    case KEY_ACCOUNTLIST_HASH:
        return ExistsInDB<DB::DBAccountList, AB::Entities::AccountList>(data);
    case KEY_CHARACTERLIST_HASH:
        return ExistsInDB<DB::DBCharacterList, AB::Entities::CharacterList>(data);
    case KEY_SERVICE_HASH:
        return ExistsInDB<DB::DBService, AB::Entities::Service>(data);
    case KEY_SERVICELIST_HASH:
        return ExistsInDB<DB::DBServicelList, AB::Entities::ServiceList>(data);
    case KEY_GUILD_HASH:
        return ExistsInDB<DB::DBGuild, AB::Entities::Guild>(data);
    case KEY_GUILDMEMBERS_HASH:
        return ExistsInDB<DB::DBGuildMembers, AB::Entities::GuildMembers>(data);
    case KEY_RESERVEDNAME_HASH:
        return ExistsInDB<DB::DBReservedName, AB::Entities::ReservedName>(data);
    case KEY_ITEMS_HASH:
        return ExistsInDB<DB::DBItem, AB::Entities::Item>(data);
    case KEY_ITEMLIST_HASH:
        return ExistsInDB<DB::DBItemList, AB::Entities::ItemList>(data);
    case KEY_VERSIONLIST_HASH:
        return ExistsInDB<DB::DBVersionList, AB::Entities::VersionList>(data);
    case KEY_ACCOUNTKEYLIST_HASH:
        return ExistsInDB<DB::DBAccountKeyList, AB::Entities::AccountKeyList>(data);
    case KEY_MUSIC_HASH:
        return ExistsInDB<DB::DBMusic, AB::Entities::Music>(data);
    case KEY_MUSICLIST_HASH:
        return ExistsInDB<DB::DBMusicList, AB::Entities::MusicList>(data);
    case KEY_CONCRETEITEMS_HASH:
        return ExistsInDB<DB::DBConcreteItem, AB::Entities::ConcreteItem>(data);
    case KEY_ACCOUNTITEMLIST_HASH:
    case KEY_CHESTITEMLIST_HASH:
        return ExistsInDB<DB::DBAccountItemList, AB::Entities::AccountItemList>(data);
    case KEY_PLAYERITEMLIST_HASH:
    case KEY_INVENTORYITEMLIST_HASH:
    case KEY_EQUIPPEDITEMLIST_HASH:
        return ExistsInDB<DB::DBPlayerItemList, AB::Entities::PlayerItemList>(data);
    case KEY_ITEMCHANCELIST_HASH:
        return ExistsInDB<DB::DBItemChanceList, AB::Entities::ItemChanceList>(data);
    case KEY_GAMEINSTANCES_HASH:
    case KEY_PARTIES_HASH:
        // Not written to DB. If we are here its not in cache so does not exist
        return false;
    default:
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

    size_t tableHash = Utils::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        auto _data = cache_.find(key);
        if (_data == cache_.end())
            return;

        AB::Entities::Character ch;
        if (GetEntity(*(*_data).second.second, ch))
        {
            auto it = playerNames_.find(ch.name);
            if (it != playerNames_.end())
                playerNames_.erase(it);
        }
    }
}
