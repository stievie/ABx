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

#pragma warning(push)
#pragma warning(disable: 4307)
static constexpr size_t KEY_ACCOUNTS_HASH = Utils::StringHash(AB::Entities::KEY_ACCOUNTS);
static constexpr size_t KEY_CHARACTERS_HASH = Utils::StringHash(AB::Entities::KEY_CHARACTERS);
static constexpr size_t KEY_GAMES_HASH = Utils::StringHash(AB::Entities::KEY_GAMES);
#pragma warning(pop)

StorageProvider::StorageProvider(size_t maxSize) :
    maxSize_(maxSize),
    currentSize_(0),
    cache_()
{
    evictor_ = std::make_shared<OldestInsertionEviction>();
}

uint32_t StorageProvider::Create(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string table;
    uint32_t _id;
    if (!DecodeKey(key, table, _id))
        return 0;

    uint32_t id = CreateData(key, data);
    std::vector<uint8_t> newKey = EncodeKey(table, id);
    CacheData(newKey, data, false);
    return id;
}

bool StorageProvider::Update(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string table;
    uint32_t id;
    if (!DecodeKey(key, table, id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return false;
    }

    if (id == 0)
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
    cache_[keyString] = { modified, data };
}

std::shared_ptr<std::vector<uint8_t>> StorageProvider::Read(const std::vector<uint8_t>& key)
{
    std::string keyString(key.begin(), key.end());//TODO think about the cost here

    auto data = cache_.find(keyString);
    if (data == cache_.end())
    {
        std::shared_ptr<std::vector<uint8_t>> d = LoadData(key);
        if (d)
            CacheData(key, d, false);
        return d;
    }

    return (*data).second.second;
}

bool StorageProvider::Delete(const std::vector<uint8_t>& key)
{
    // Delete from DB
    DeleteData(key);
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

bool StorageProvider::DecodeKey(const std::vector<uint8_t>& key, std::string& table, uint32_t& id)
{
    // key = <tablename><id>
    if (key.size() <= sizeof(uint32_t))
        return false;
    table.assign(key.begin(), key.end() - sizeof(uint32_t));
    size_t start = key.size() - sizeof(uint32_t);
    id = (key[start + 3] << 24) | (key[start + 2] << 16) | (key[start + 1] << 8) | key[start];
    return false;
}

std::vector<uint8_t> StorageProvider::EncodeKey(const std::string& table, uint32_t id)
{
    std::vector<uint8_t> result(table.begin(), table.end());
    result.push_back((uint8_t)id);
    result.push_back((uint8_t)(id >> 8));
    result.push_back((uint8_t)(id >> 16));
    result.push_back((uint8_t)(id >> 24));
    return result;
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

uint32_t StorageProvider::CreateData(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string table;
    uint32_t id;
    if (!DecodeKey(key, table, id))
        return 0;

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

std::shared_ptr<std::vector<uint8_t>> StorageProvider::LoadData(const std::vector<uint8_t>& key)
{
    std::string table;
    uint32_t id;
    if (!DecodeKey(key, table, id))
        return std::shared_ptr<std::vector<uint8_t>>();

    size_t tableHash = Utils::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTS_HASH:
        return LoadFromDB<DB::DBAccount, AB::Entities::Account>(id);
    case KEY_CHARACTERS_HASH:
        return LoadFromDB<DB::DBCharacter, AB::Entities::Character>(id);
    case KEY_GAMES_HASH:
        return LoadFromDB<DB::DBGame, AB::Entities::Game>(id);
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }

    return std::shared_ptr<std::vector<uint8_t>>();
}

void StorageProvider::FlushData(const std::vector<uint8_t>& key)
{
    std::string keyString(key.begin(), key.end());//TODO think about the cost here

    auto data = cache_.find(keyString);
    if (data == cache_.end())
        return;
    // No need to save to DB when not modified
    if (!(*data).second.first)
        return;

    std::string table;
    uint32_t id;
    if (!DecodeKey(key, table, id))
        return;

    auto d = (*data).second.second.get();
    size_t tableHash = Utils::StringHashRt(table.data());
    bool succ = false;
    switch (tableHash)
    {
    case KEY_ACCOUNTS_HASH:
        succ = SaveToDB<DB::DBAccount, AB::Entities::Account>(*d);
        if (!succ)
            LOG_ERROR << "Unable to store Account data" << std::endl;
        break;
    case KEY_CHARACTERS_HASH:
        succ = SaveToDB<DB::DBCharacter, AB::Entities::Character>(*d);
        if (!succ)
            LOG_ERROR << "Unable to store Character data" << std::endl;
        break;
    case KEY_GAMES_HASH:
        succ = SaveToDB<DB::DBGame, AB::Entities::Game>(*d);
        if (!succ)
            LOG_ERROR << "Unable to store Game data" << std::endl;
        break;
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }

    if (succ)
        (*data).second.first = false;
}

void StorageProvider::DeleteData(const std::vector<uint8_t>& key)
{
    std::string table;
    uint32_t id;
    if (!DecodeKey(key, table, id))
        return;

    auto data = Read(key);
    if (!data)
        return;
    std::vector<uint8_t>* d = data.get();
    size_t tableHash = Utils::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTS_HASH:
        if (!DeleteFromDB<DB::DBAccount, AB::Entities::Account>(*d))
            LOG_ERROR << "Unable to delete Account data" << std::endl;
        break;
    case KEY_CHARACTERS_HASH:
        if (!DeleteFromDB<DB::DBCharacter, AB::Entities::Character>(*d))
            LOG_ERROR << "Unable to delete Character data" << std::endl;
        break;
    case KEY_GAMES_HASH:
        if (!DeleteFromDB<DB::DBGame, AB::Entities::Game>(*d))
            LOG_ERROR << "Unable to delete Character data" << std::endl;
        break;
    default:
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }

}
