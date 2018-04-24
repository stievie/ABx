#include "stdafx.h"
#include "StorageProvider.h"
#include "Database.h"
#include <sstream>
#include <AB/Entities/Account.h>
#include <AB/Entities/Character.h>
#include <AB/Entities/Game.h>
#include "DBAccount.h"
#include "DBCharacter.h"
#include "DBGame.h"
#include "StringHash.h"
#include "Logger.h"
#include "StringHash.h"
#include <AB/Entities/Limits.h>
#include "Scheduler.h"
#include "Dispatcher.h"

#pragma warning(push)
#pragma warning(disable: 4307)
static constexpr size_t KEY_ACCOUNTS_HASH = Utils::StringHash(AB::Entities::Account::KEY());
static constexpr size_t KEY_CHARACTERS_HASH = Utils::StringHash(AB::Entities::Character::KEY());
static constexpr size_t KEY_GAMES_HASH = Utils::StringHash(AB::Entities::Game::KEY());
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
    CacheData(key, data, true, false);
    // Unfortunately we must flush the data for create operations. Or we find a way
    // to check constraints, unique columns etc.
    if (!FlushData(key))
    {
        // Failed -> remove from cache
        RemoveData(keyString);
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
    CacheData(key, data, true, isCreated);
    return true;
}

void StorageProvider::CacheData(const std::vector<uint8_t>& key,
    std::shared_ptr<std::vector<uint8_t>> data, bool modified, bool created)
{
    size_t sizeNeeded = data->size();
    if (!EnoughSpace(sizeNeeded))
    {
        // Create space later
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(10, std::bind(&StorageProvider::CreateSpace, this, sizeNeeded))
        );
    }

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
}

bool StorageProvider::Read(const std::vector<uint8_t>& key,
    std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string keyString(key.begin(), key.end());

    auto _data = cache_.find(keyString);
    if (_data == cache_.end())
    {
        std::string table;
        uuids::uuid _id;
        if (!DecodeKey(key, table, _id))
        {
            LOG_ERROR << "Unable to decode key" << std::endl;
            return false;
        }

        if (!LoadData(key, data))
            return false;

        if (_id.nil())
            // If no UUID given in key (e.g. when reading by name) cache with the proper key
            _id = GetUuid(data);
        auto newKey = EncodeKey(table, _id);
        CacheData(newKey, data, false, true);
        return true;
    }

    if ((*_data).second.first.deleted)
        // Don't return deleted items that are in cache
        return false;
//    data = (*_data).second.second;
    data->assign((*_data).second.second->begin(), (*_data).second.second->end());
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
    std::string dataToRemove(key.begin(), key.end());
    if (!FlushData(key))
        return false;
    return RemoveData(dataToRemove);
}

bool StorageProvider::Preload(const std::vector<uint8_t>& key)
{
    std::string keyString(key.begin(), key.end());

    auto _data = cache_.find(keyString);
    if (_data == cache_.end())
        return true;

    std::string table;
    uuids::uuid _id;
    if (!DecodeKey(key, table, _id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return false;
    }

    std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>(0);
    if (!LoadData(key, data))
        return false;

    if (_id.nil())
        // If no UUID given in key (e.g. when reading by name) cache with the proper key
        _id = GetUuid(data);
    auto newKey = EncodeKey(table, _id);
    CacheData(newKey, data, false, true);
    return true;
}

void StorageProvider::Shutdown()
{
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
    if (cache_.size() == 0)
        return;
    std::lock_guard<std::mutex> lock(lock_);
    size_t oldSize = currentSize_;
    int removed = 0;
    auto i = cache_.begin();
    while ((i = std::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        return current.second.first.deleted;
    })) != cache_.end())
    {
        ++removed;
        std::vector<uint8_t> key((*i).first.begin(), (*i).first.end());
        FlushData(key);
        currentSize_ -= (*i).second.second->size();
        evictor_->DeleteKey((*i).first);
        cache_.erase(i++);
    }
    if (removed > 0)
    {
        LOG_INFO << "Cleaned cache old size " << oldSize << " current size " << currentSize_ <<
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
    std::lock_guard<std::mutex> lock(lock_);
    auto i = cache_.begin();
    while ((i = std::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        return (current.second.first.modified || !current.second.first.created) &&
            !!current.second.first.deleted;
    })) != cache_.end())
    {
        ++written;
        std::vector<uint8_t> key((*i).first.begin(), (*i).first.end());
        FlushData(key);
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

uuids::uuid StorageProvider::GetUuid(std::shared_ptr<std::vector<uint8_t>> data)
{
    // Get UUID from raw data. UUID is serialized first as string
    const std::string suuid(data->begin() + 1,
        data->begin() + AB::Entities::Limits::MAX_UUID + 1);
    return uuids::uuid(suuid);
}

bool StorageProvider::EnoughSpace(size_t size)
{
    return (currentSize_ + size) <= maxSize_;
}

void StorageProvider::CreateSpace(size_t size)
{
    // Create more than required space
    std::lock_guard<std::mutex> lock(lock_);
    const size_t sizeNeeded = size * 2;
    while ((currentSize_ + sizeNeeded) > maxSize_)
    {
        std::string dataToRemove = evictor_->NextEviction();
        std::vector<uint8_t> key(dataToRemove.begin(), dataToRemove.end());

        FlushData(key);
        RemoveData(dataToRemove);
    }
}

bool StorageProvider::RemoveData(const std::string& key)
{
    auto data = cache_.find(key);
    if (data != cache_.end())
    {
        currentSize_ -= (*data).second.second->size();
        cache_.erase(key);
        evictor_->DeleteKey(key);
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
        return false;
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
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        return false;
    }

    if (!succ)
        LOG_ERROR << "Unable to write data" << std::endl;
    return succ;
}
