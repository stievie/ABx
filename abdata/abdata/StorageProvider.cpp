#include "stdafx.h"
#include "StorageProvider.h"
#include <string>
#include "Database.h"

StorageProvider::StorageProvider(size_t maxSize) :
    maxSize_(maxSize),
    currentSize_(0),
    cache_()
{
    evictor_.reset(new OldestInsertionEviction());
}

void StorageProvider::Save(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data)
{
    std::string keyString(key.begin(), key.end());//TODO think about the cost here
    if (!EnoughSpace(data->size()))
    {
        CreateSpace(data->size());
    }

    //we check if its already in cache
    if (cache_.find(keyString) == cache_.end())
    {
        evictor_->AddKey(keyString);
        currentSize_ += data->size();
    }
    else
    {
        evictor_->RefreshKey(keyString);
        currentSize_ = (currentSize_ - cache_[keyString]->size()) + data->size();
    }
    cache_[keyString] = data;
}

std::shared_ptr<std::vector<uint8_t>> StorageProvider::Get(const std::vector<uint8_t>& key)
{
    std::string keyString(key.begin(), key.end());//TODO think about the cost here

    auto data = cache_.find(keyString);
    if (data == cache_.end())
    {
        std::shared_ptr<std::vector<uint8_t>> d = LoadData(key);
        if (d)
            Save(key, d);
        return d;
    }

    return (*data).second;
}

bool StorageProvider::Remove(const std::vector<uint8_t>& key)
{
    std::string dataToRemove(key.begin(), key.end());
    return RemoveData(dataToRemove);
}

bool StorageProvider::DecodeKey(const std::vector<uint8_t>& key, std::string& table, uint32_t& id)
{
    // key = <tablename><id>
    if (key.size() <= sizeof(uint32_t))
        return false;
    table.assign(key.begin(), key.end() + sizeof(uint32_t));
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
        RemoveData(dataToRemove);
    }
}

bool StorageProvider::RemoveData(const std::string& key)
{
    auto data = cache_.find(key);
    if (data != cache_.end())
    {
        currentSize_ -= (*data).second->size();
        cache_.erase(key);
        evictor_->DeleteKey(key);
        return true;
    }
    return false;
}

std::shared_ptr<std::vector<uint8_t>> StorageProvider::LoadData(const std::vector<uint8_t>& key)
{
    std::string table;
    uint32_t id;
    if (!DecodeKey(key, table, id))
        return std::shared_ptr<std::vector<uint8_t>>();
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM " << table << " WHERE `id` = " << id;
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return std::shared_ptr<std::vector<uint8_t>>();

    return std::shared_ptr<std::vector<uint8_t>>();
}


