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

#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include "Asset.h"
#include "IOAsset.h"
#include "Logger.h"
#include "FileUtils.h"
#include <unordered_map>
#include "StringUtils.h"
#include <sa/TypeName.h>
#include <sa/StringHash.h>

namespace IO {

typedef std::pair<size_t, std::string> CacheKey;

struct KeyHasher
{
    size_t operator()(const CacheKey& s) const noexcept
    {
        // https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
        // Compute individual hash values for first and second
        // http://stackoverflow.com/a/1646913/126995
        size_t res = 17;
        // The first is already a hash
        res = res * 31 + s.first;
        res = res * 31 + std::hash<std::string>()(s.second);
        return res;
    }
};

class DataProvider
{
private:
    std::map<size_t, std::unique_ptr<IOAsset>> importers_;
    std::unordered_map<CacheKey, std::shared_ptr<Asset>, KeyHasher> cache_;
public:
    DataProvider();
    ~DataProvider() = default;

    std::string GetDataFile(const std::string& name) const;
    const std::string& GetDataDir() const;
    bool FileExists(const std::string& name) const;
    std::string GetFile(const std::string& name) const;

    /// Check if an asset exists in cache or as file
    template<class T>
    bool Exists(const std::string& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        constexpr size_t hash = sa::StringHash(sa::TypeName<T>::Get());
        const CacheKey key = std::make_pair(hash, normal_name);
        auto it = cache_.find(key);
        if (it != cache_.end())
            return true;
        return FileExists(name);
    }
    template<class T>
    bool IsCached(const std::string& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        constexpr size_t hash = sa::StringHash(sa::TypeName<T>::Get());
        const CacheKey key = std::make_pair(hash, normal_name);
        auto it = cache_.find(key);
        return (it != cache_.end());
    }
    /// Add an asset to the cache
    /// @return true if it was added, false if such an asset already exists in the cache
    template<class T>
    bool AddToCache(std::shared_ptr<T> asset, const std::string& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        constexpr size_t hash = sa::StringHash(sa::TypeName<T>::Get());
        const CacheKey key = std::make_pair(hash, normal_name);
        auto it = cache_.find(key);
        if (it != cache_.end())
            return false;
        cache_[key] = asset;
        return true;
    }
    template<class T>
    bool RemoveFromCache(std::shared_ptr<T> asset, const std::string& name)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        constexpr size_t hash = sa::StringHash(sa::TypeName<T>::Get());
        const CacheKey key = std::make_pair(hash, normal_name);
        auto it = cache_.find(key);
        if (it != cache_.end() && (*it).second.get() == asset.get())
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
        constexpr size_t hash = sa::StringHash(sa::TypeName<T>::Get());
        const CacheKey key = std::make_pair(hash, normal_name);
        auto it = cache_.find(key);
        if (it != cache_.end())
        {
            return std::static_pointer_cast<T>((*it).second);
        }
        return std::shared_ptr<T>();
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
        constexpr size_t hash = sa::StringHash(sa::TypeName<T>::Get());
        importers_[hash] = std::make_unique<I>();
    }
    /// Get an importer instance
    template<class T>
    IOAsset* GetImporter()
    {
        constexpr size_t hash = sa::StringHash(sa::TypeName<T>::Get());
        auto it = importers_.find(hash);
        if (it != importers_.end())
        {
            return (*it).second.get();
        }
        return nullptr;
    }
    template<class T>
    bool Import(T& asset, const std::string& name)
    {
        const std::string file = GetFile(name);
        if (!FileExists(file))
        {
            LOG_WARNING << "File not found " << name << std::endl;
            return false;
        }

        IOAssetImpl<T>* imp = static_cast<IOAssetImpl<T>*>(GetImporter<T>());
        if (!imp)
        {
            LOG_WARNING << "No importer found for " << name << std::endl;
            return false;
        }
        bool r = imp->Import(asset, file);
        if (r)
            asset.SetFileName(file);
        return r;
    }

    template<class T>
    std::shared_ptr<T> GetAsset(const std::string& name, bool cacheAble = true)
    {
        const std::string normal_name = GetFile(Utils::NormalizeFilename(name));
        constexpr size_t hash = sa::StringHash(sa::TypeName<T>::Get());
        const CacheKey key = std::make_pair(hash, normal_name);
        // Lookup in cache
        if (cacheAble)
        {
            auto it = cache_.find(key);
            if (it != cache_.end())
            {
                return std::static_pointer_cast<T>((*it).second);
            }
        }

        // Not in cache make a new one and load it
        std::shared_ptr<T> asset = std::make_shared<T>();
        assert(asset);
        if (Import<T>(*asset, normal_name))
        {
            if (cacheAble)
                cache_[key] = asset;
            return asset;
        }
        return std::shared_ptr<T>();
    }
};

}

