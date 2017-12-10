#pragma once

#include <string>
#include "Asset.h"
#include <functional>
#include <map>
#include "IOAsset.h"
#include "Logger.h"
#include "Utils.h"

namespace IO {

class DataProvider
{
private:
    std::map<const char*, std::unique_ptr<IOAsset>> importers_;
    std::map<std::pair<const char*, std::string>, std::shared_ptr<Asset>> cache_;
public:
    DataProvider();
    ~DataProvider() = default;

    std::string GetDataFile(const std::string& name) const;
    bool FileExists(const std::string& name) const;
    std::string GetFile(const std::string& name) const;

    /// Check if an asset exists in cache or as file
    template<class T>
    bool Exists(const std::wstring& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        const std::pair<const char*, std::string> key = std::make_pair(typeid(T).name(), normal_name);
        auto it = cache_.find(key);
        if (it != cache_.end())
            return true;
        return FileExists(name);
    }
    template<class T>
    bool IsCached(const std::wstring& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        const std::pair<const char*, std::string> key = std::make_pair(typeid(T).name(), normal_name);
        auto it = cache_.find(key);
        return (it != cache_.end());
    }
    /// Add an asset to the cache
    /// @return true if it was added, false if such an asset already exists in the cache
    template<class T>
    bool AddToCache(AssetImpl<T>* asset, const std::string& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        const std::pair<const char*, std::string> key = std::make_pair(typeid(T).name(), normal_name);
        auto it = cache_.find(key);
        if (it != cache_.end())
            return false;
        cache_[key] = asset->GetPtr();
        return true;
    }
    template<class T>
    bool RemoveFromCache(AssetImpl<T>* asset, const std::string& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        const std::pair<const char*, std::string> key = std::make_pair(typeid(T).name(), normal_name);
        auto it = cache_.find(key);
        if (it != cache_.end() && (*it).second.get() == asset)
        {
            cache_.erase(it);
            return true;
        }
        return false;
    }
    template<class T>
    std::shared_ptr<T> GetCached(const std::string& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        const std::pair<const char*, std::string> key = std::make_pair(typeid(T).name(), normal_name);
        auto it = cache_.find(key);
        if (it != cache_.end())
        {
            AssetImpl<T>* a = static_cast<AssetImpl<T>*>((*it).second.get());
            return a->GetPtr();
        }
        return nullptr;
    }
    /// Remove all objects from the cache
    void ClearCache()
    {
        cache_.clear();
    }
    /// Remove all objects that are only referenced by the cache, i.e. nobody
    /// else owns it anymore.
    void CleanCache();

    /// Add an importer class
    template<class T, class I>
    void AddImporter()
    {
        importers_[typeid(T).name()] = std::make_unique<I>();
    }
    /// Get an importer instance
    template<class T>
    IOAsset* GetImporter()
    {
        auto it = importers_.find(typeid(T).name());
        if (it != importers_.end())
        {
            return (*it).second.get();
        }
        return nullptr;
    }
    template<class T>
    bool Import(T* asset, const std::string& name)
    {
        const std::string file = GetFile(name);
        if (FileExists(file))
        {
            IOAssetImpl<T>* imp = static_cast<IOAssetImpl<T>*>(GetImporter<T>());
            if (!imp)
            {
                LOG_WARNING << "No importer found for " << name;
                return false;
            }
            asset->fileName_ = file;
            return imp->Import(asset, file);
        }
        else
            LOG_WARNING << "File not found " << name;
        return false;
    }

    template<class T>
    std::shared_ptr<T> GetAsset(const std::string& name, bool cacheAble = true)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        const std::pair<const char*, std::string> key = std::make_pair(typeid(T).name(), normal_name);
        // Lookup in cache
        if (cacheAble)
        {
            auto it = cache_.find(key);
            if (it != cache_.end())
            {
                AssetImpl<T>* a = static_cast<AssetImpl<T>*>((*it).second.get());
                return a->GetPtr();
            }
        }

        // Not in cache make a new one and load it
        std::shared_ptr<T> asset = std::make_shared<T>();
        if (Import<T>(asset.get(), normal_name))
        {
            if (cacheAble)
                cache_[key] = asset;
            return asset;
        }
        return nullptr;
    }

    static DataProvider Instance;
};

}
