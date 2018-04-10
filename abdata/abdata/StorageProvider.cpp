#include "stdafx.h"
#include "StorageProvider.h"
#include <string>

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
        return 0;

    return (*data).second;
}

int StorageProvider::Remove(const std::vector<uint8_t>& key)
{
    std::string dataToRemove(key.begin(), key.end());
    return RemoveData(dataToRemove);
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


