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

#pragma warning(push)
#pragma warning(disable: 4307)
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
#pragma warning(pop)

StorageProvider::StorageProvider(size_t maxSize, bool readonly) :
    maxSize_(maxSize),
    currentSize_(0),
    readonly_(readonly),
    running_(true),
    cache_()
{
    evictor_ = std::make_unique<OldestInsertionEviction>();
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(FLUSH_CACHE_MS, std::bind(&StorageProvider::FlushCacheTask, this))
    );
    Asynch::Scheduler::Instance.Add(
        Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&StorageProvider::CleanCacheTask, this))
    );
}

bool StorageProvider::Create(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string keyString(key.begin(), key.end());
    auto _data = cache_.find(keyString);

    if (_data != cache_.end())
    {
        if ((*_data).second.first.deleted)
            FlushData(key);
        else
            // Already exists
            return false;
    }

    std::string table;
    uuids::uuid _id;
    if (!DecodeKey(key, table, _id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return false;
    }
    if (_id.nil())
        return false;

    // Mark modified since not in DB
    CacheData(table, _id, data, true, false);
    // Unfortunately we must flush the data for create operations. Or we find a way
    // to check constraints, unique columns etc.
    if (!FlushData(key))
    {
        // Failed -> remove from cache
        RemoveData(key);
        return false;
    }
    return true;
}

bool StorageProvider::Update(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string table;
    uuids::uuid id;
    if (!DecodeKey(key, table, id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return false;
    }

    if (id.nil())
        // Does not exist
        return false;

    std::string keyString(key.begin(), key.end());
    auto _data = cache_.find(keyString);
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
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(std::bind(&StorageProvider::CreateSpace, this, sizeNeeded))
        );
    }

    auto key = EncodeKey(table, id);

    std::string keyString(key.begin(), key.end());
    // we check if its already in cache
    if (cache_.find(keyString) == cache_.end())
    {
        evictor_->AddKey(keyString);
        currentSize_ += data->size();
    }
    else
    {
        evictor_->RefreshKey(keyString);
        currentSize_ = (currentSize_ - cache_[keyString].second->size()) + data->size();
    }

    cache_[keyString] = { { created, modified, false }, data };

    // Special case for player names
    size_t tableHash = Utils::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        AB::Entities::Character ch;
        if (GetEntity(*data, ch))
        {
            playerNames_[ch.name] = ch.uuid;
        }
    }
}

bool StorageProvider::Read(const std::vector<uint8_t>& key,
    std::shared_ptr<std::vector<uint8_t>> data)
{
//    AB_PROFILE;
    std::string keyString(key.begin(), key.end());

    auto _data = cache_.find(keyString);
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
    if (!DecodeKey(key, table, _id))
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
            auto playerIt = playerNames_.find(ch.name);
            if (playerIt != playerNames_.end())
            {
                std::string playerUuid = (*playerIt).second;
                auto playerKey = EncodeKey(table, uuids::uuid(playerUuid));

                std::string playerKeyString(playerKey.begin(), playerKey.end());

                _data = cache_.find(playerKeyString);
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
    auto newKey = EncodeKey(table, _id);
    std::string newKeyString(newKey.begin(), newKey.end());
    auto _newdata = cache_.find(newKeyString);
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

bool StorageProvider::Delete(const std::vector<uint8_t>& key)
{
    std::string dataToRemove(key.begin(), key.end());
    auto data = cache_.find(dataToRemove);
    if (data == cache_.end())
    {
        return false;
    }
    (*data).second.first.deleted = true;
    return true;
}

bool StorageProvider::Invalidate(const std::vector<uint8_t>& key)
{
    if (!FlushData(key))
        return false;
    return RemoveData(key);
}

void StorageProvider::PreloadTask(std::vector<uint8_t> key)
{
    // Dispatcher thread

    std::string keyString(key.begin(), key.end());

    auto _data = cache_.find(keyString);
    if (_data == cache_.end())
        return;

    std::string table;
    uuids::uuid _id;
    if (!DecodeKey(key, table, _id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return;
    }
    std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>(0);
    if (!LoadData(key, data))
        return;

    if (_id.nil())
        // If no UUID given in key (e.g. when reading by name) cache with the proper key
        _id = GetUuid(*data);
    auto newKey = EncodeKey(table, _id);

    std::string newKeyString(newKey.begin(), newKey.end());
    auto _newdata = cache_.find(newKeyString);
    if (_newdata == cache_.end())
    {
        CacheData(table, _id, data, false, true);
    }
}

bool StorageProvider::Preload(const std::vector<uint8_t>& key)
{
    std::string keyString(key.begin(), key.end());

    auto _data = cache_.find(keyString);
    if (_data == cache_.end())
        return true;

    // Load later
    Asynch::Dispatcher::Instance.Add(
        Asynch::CreateTask(std::bind(&StorageProvider::PreloadTask, this, key))
    );
    return true;
}

bool StorageProvider::Exists(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string keyString(key.begin(), key.end());
    auto _data = cache_.find(keyString);

    if (_data != cache_.end())
        return !(*_data).second.first.deleted;

    return ExistsData(key, *data);
}

void StorageProvider::Shutdown()
{
    // The only thing that not called from the dispatcher thread, so lock it.
    std::lock_guard<std::mutex> lock(lock_);
    running_ = false;
    for (const auto& c : cache_)
    {
        std::vector<uint8_t> key(c.first.begin(), c.first.end());
        FlushData(key);
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
        bool error = false;
        if ((*i).second.first.created)
        {
            // No need to delete from DB when it's not in DB
            std::vector<uint8_t> key((*i).first.begin(), (*i).first.end());
            error = FlushData(key);
        }
        if (!error)
        {
            currentSize_ -= (*i).second.second->size();
            evictor_->DeleteKey((*i).first);
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

    // Clean player names
    while (playerNames_.size() > MAX_PLAYERNAMES_CACHE)
        playerNames_.erase(playerNames_.begin());

    if (removed > 0)
    {
        LOG_INFO << "Cleaned cache old size " << Utils::ConvertSize(oldSize) <<
            " current size " << Utils::ConvertSize(currentSize_) <<
            " removed " << removed << " record(s)" << std::endl;
    }
}

void StorageProvider::CleanCacheTask()
{
    CleanCache();
    if (running_)
    {
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&StorageProvider::CleanCacheTask, this))
        );
    }
}

void StorageProvider::FlushCache()
{
    if (cache_.size() == 0)
        return;
    int written = 0;
    auto i = cache_.begin();
    while ((i = std::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        // Don't return deleted, these are flushed in CleanCache()
        return (current.second.first.modified || !current.second.first.created) &&
            !current.second.first.deleted;
    })) != cache_.end())
    {
        ++written;
        std::vector<uint8_t> key((*i).first.begin(), (*i).first.end());
        if (!FlushData(key))
            // Error, break for now and try  the next time.
            // In case of lost connection it would try forever.
            break;
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
        Asynch::Scheduler::Instance.Add(
            Asynch::CreateScheduledTask(FLUSH_CACHE_MS, std::bind(&StorageProvider::FlushCacheTask, this))
        );
    }
}

bool StorageProvider::DecodeKey(const std::vector<uint8_t>& key,
    std::string& table, uuids::uuid& id)
{
    // key = <tablename><guid>
    if (key.size() <= uuids::uuid::state_size)
        return false;
    table.assign(key.begin(), key.end() - uuids::uuid::state_size);
    id = uuids::uuid(key.end() - uuids::uuid::state_size, key.end());
    return true;
}

std::vector<uint8_t> StorageProvider::EncodeKey(const std::string& table, const uuids::uuid& id)
{
    std::vector<uint8_t> result(table.begin(), table.end());
    result.insert(result.end(), id.begin(), id.end());
    return result;
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
        std::string dataToRemove = evictor_->NextEviction();
        std::vector<uint8_t> key(dataToRemove.begin(), dataToRemove.end());

        if (FlushData(key))
            RemoveData(key);
        else
            break;
    }
}

bool StorageProvider::RemoveData(const std::vector<uint8_t>& key)
{
    std::string strKey(key.begin(), key.end());
    auto data = cache_.find(strKey);
    if (data != cache_.end())
    {
        currentSize_ -= (*data).second.second->size();
        cache_.erase(strKey);
        evictor_->DeleteKey(strKey);

        return true;
    }
    return false;
}

bool StorageProvider::LoadData(const std::vector<uint8_t>& key,
    std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string table;
    uuids::uuid id;
    if (!DecodeKey(key, table, id))
        return false;

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
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }

    return false;
}

bool StorageProvider::FlushData(const std::vector<uint8_t>& key)
{
    if (readonly_)
    {
        LOG_WARNING << "READONLY: Nothing is written to the database" << std::endl;
        return true;
    }

    std::string keyString(key.begin(), key.end());

    auto data = cache_.find(keyString);
    if (data == cache_.end())
        return false;
    // No need to save to DB when not modified
    if (!(*data).second.first.modified && !(*data).second.first.deleted)
        return true;

    std::string table;
    uuids::uuid id;
    if (!DecodeKey(key, table, id))
        return false;

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
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        return false;
    }

    if (!succ)
        LOG_ERROR << "Unable to write data" << std::endl;
    return succ;
}

bool StorageProvider::ExistsData(const std::vector<uint8_t>& key, std::vector<uint8_t>& data)
{
    std::string table;
    uuids::uuid id;
    if (!DecodeKey(key, table, id))
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
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }

    return false;
}
