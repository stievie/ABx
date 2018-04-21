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
    cache_()
{
    evictor_ = std::make_shared<OldestInsertionEviction>();
}

bool StorageProvider::Create(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string keyString(key.begin(), key.end());//TODO think about the cost here
    auto _data = cache_.find(keyString);
    if (_data != cache_.end())
        // Already exists
        return false;

    std::string table;
    uuids::uuid _id;
    if (!DecodeKey(key, table, _id))
        return false;
    if (_id.nil())
        return false;

    // Mark modified since to in DB
    CacheData(key, data, true);
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

    // The client sets the data so this is not stored in DB
    CacheData(key, data, true);
    return true;
}

void StorageProvider::CacheData(const std::vector<uint8_t>& key,
    std::shared_ptr<std::vector<uint8_t>> data, bool modified)
{
    std::string keyString(key.begin(), key.end());//TODO think about the cost here

    if (!EnoughSpace(data->size()))
        CreateSpace(data->size());

    //we check if its already in cache
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
    cache_[keyString] = { { modified, false }, data };
}

bool StorageProvider::Read(const std::vector<uint8_t>& key,
    std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string keyString(key.begin(), key.end());//TODO think about the cost here

    auto _data = cache_.find(keyString);
    if (_data == cache_.end())
    {
        if (!LoadData(key, data))
            return false;

        std::string table;
        uuids::uuid _id;
        DecodeKey(key, table, _id);
        if (_id.nil())
            // If no UUID given in key (e.g. when reading by name) cache with the proper key
            _id = GetUuid(data);
        auto newKey = EncodeKey(table, _id);
        CacheData(newKey, data, false);
        return true;
    }

    if ((*_data).second.first.deleted)
        // Don't return deleted items that are in cache
        return false;
    data->assign((*_data).second.second->begin(), (*_data).second.second->end());
    return true;
}

bool StorageProvider::Delete(const std::vector<uint8_t>& key)
{
    std::string dataToRemove(key.begin(), key.end());
    auto data = cache_.find(dataToRemove);
    if (data != cache_.end())
    {
        (*data).second.first.deleted = true;
        return true;
    }
    return false;
}

bool StorageProvider::Invalidate(const std::vector<uint8_t>& key)
{
    std::string dataToRemove(key.begin(), key.end());
    return RemoveData(dataToRemove);
}

void StorageProvider::Shutdown()
{
    for (const auto& c : cache_)
    {
        std::vector<uint8_t> key(c.first.begin(), c.first.end());
        FlushData(key);
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
    // Get UUID from raw data. First is id = uint32_t, second is the UUID as string
    const std::string suuid(data->begin() + sizeof(uint32_t) + 1,
        data->begin() + sizeof(uint32_t) + AB::Entities::Limits::MAX_UUID + 1);
    return uuids::uuid(suuid);
}

bool StorageProvider::EnoughSpace(size_t size)
{
    return (currentSize_ + size) <= maxSize_;
}

void StorageProvider::CreateSpace(size_t size)
{
    while ((currentSize_ + size) > maxSize_)
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

bool StorageProvider::CreateData(const std::vector<uint8_t>& key,
    std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string table;
    uuids::uuid id;
    if (!DecodeKey(key, table, id))
        return 0;

    if (readonly_)
    {
        LOG_WARNING << "READONLY: Nothing written to database" << std::endl;
        return false;
    }

    size_t tableHash = Utils::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTS_HASH:
        return CreateInDB<DB::DBAccount, AB::Entities::Account>(*data);
    case KEY_CHARACTERS_HASH:
        return CreateInDB<DB::DBCharacter, AB::Entities::Character>(*data);
    case KEY_GAMES_HASH:
        return CreateInDB<DB::DBGame, AB::Entities::Game>(*data);
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }
    return 0;
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

void StorageProvider::FlushData(const std::vector<uint8_t>& key)
{
    std::string keyString(key.begin(), key.end());//TODO think about the cost here

    auto data = cache_.find(keyString);
    if (data == cache_.end())
        return;
    // No need to save to DB when not modified
    if (!(*data).second.first.modified || !(*data).second.first.deleted)
        return;

    std::string table;
    uuids::uuid id;
    if (!DecodeKey(key, table, id))
        return;

    if (readonly_)
    {
        LOG_WARNING << "READONLY: Nothing written to database" << std::endl;
        return;
    }

    auto d = (*data).second.second.get();
    size_t tableHash = Utils::StringHashRt(table.data());
    bool succ = false;
    switch (tableHash)
    {
    case KEY_ACCOUNTS_HASH:
        if ((*data).second.first.modified)
            succ = SaveToDB<DB::DBAccount, AB::Entities::Account>(*d);
        else if ((*data).second.first.deleted)
            succ = DeleteFromDB<DB::DBAccount, AB::Entities::Account>(*d);
        if (!succ)
            LOG_ERROR << "Unable to flush Account data" << std::endl;
        break;
    case KEY_CHARACTERS_HASH:
        if ((*data).second.first.modified)
            succ = SaveToDB<DB::DBCharacter, AB::Entities::Character>(*d);
        else if ((*data).second.first.deleted)
            succ = DeleteFromDB<DB::DBCharacter, AB::Entities::Character>(*d);
        if (!succ)
            LOG_ERROR << "Unable to flush Character data" << std::endl;
        break;
    case KEY_GAMES_HASH:
        if ((*data).second.first.modified)
            succ = SaveToDB<DB::DBGame, AB::Entities::Game>(*d);
        else if ((*data).second.first.deleted)
            succ = DeleteFromDB<DB::DBGame, AB::Entities::Game>(*d);
        if (!succ)
            LOG_ERROR << "Unable to flush Game data" << std::endl;
        break;
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }

    if (succ)
    {
        if ((*data).second.first.modified)
            (*data).second.first.modified = false;
        else if ((*data).second.first.deleted)
            RemoveData(keyString);
    }
}
