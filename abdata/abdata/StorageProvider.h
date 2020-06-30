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

#include <mutex>
#include <vector>
#include <sa/Compiler.h>
#include "CacheIndex.h"
#include "NameIndex.h"
#include <eastl.hpp>

PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4310 4100)
    PRAGMA_WARNING_DISABLE_CLANG("-Wunused-parameter")
    PRAGMA_WARNING_DISABLE_GCC("-Wunused-parameter")
#   include <bitsery/bitsery.h>
#   include <bitsery/adapter/buffer.h>
#   include <bitsery/traits/vector.h>
#   include <bitsery/traits/string.h>
PRAGMA_WARNING_POP

#include <sa/StringHash.h>
#include <abscommon/DataKey.h>
#include <sa/CallableTable.h>
#include <sa/Bits.h>

// Clean cache every 10min
#define CLEAN_CACHE_MS (1000 * 60 * 10)
// Flush cache every minute
#define FLUSH_CACHE_MS (1000 * 60)
// Clear prices all 5sec, is this a good value?
#define CLEAR_PRICES_MS (1000 * 5)

struct CacheFlag
{
    enum
    {
        Created = 1,              // It exists in DB
        Modified = 1 << 1,
        Deleted = 1 << 2,
    };
};

using CacheFlags = uint32_t;

inline bool IsCreated(CacheFlags flags)
{
    return sa::bits::is_set(flags, CacheFlag::Created);
}
inline bool IsModified(CacheFlags flags)
{
    return sa::bits::is_set(flags, CacheFlag::Modified);
}
inline bool IsDeleted(CacheFlags flags)
{
    return sa::bits::is_set(flags, CacheFlag::Deleted);
}

using StorageData = std::vector<uint8_t>;

class StorageProvider
{
public:
    StorageProvider(size_t maxSize, bool readonly);

    bool Lock(uint32_t clientId, const IO::DataKey& key);
    bool Unlock(uint32_t clientId, const IO::DataKey& key);
    bool Create(uint32_t clientId, const IO::DataKey& key, ea::shared_ptr<StorageData> data);
    bool Update(uint32_t clientId, const IO::DataKey& key, ea::shared_ptr<StorageData> data);
    bool Read(uint32_t clientId, const IO::DataKey& key, ea::shared_ptr<StorageData> data);
    bool Delete(uint32_t clientId, const IO::DataKey& key);
    bool Invalidate(uint32_t clientId, const IO::DataKey& key);
    bool Preload(uint32_t clientId, const IO::DataKey& key);
    bool Exists(uint32_t clientId, const IO::DataKey& key, ea::shared_ptr<StorageData> data);
    bool Clear(uint32_t clientId, const IO::DataKey& key);

    // Client compatible Methods
    template<typename E>
    bool EntityRead(E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        ea::shared_ptr<StorageData> data = ea::make_shared<StorageData>();
        SetEntity<E>(entity, *data);
        if (!Read(MY_CLIENT_ID, aKey, data))
            return false;
        if (GetEntity(*data.get(), entity))
            return true;
        return false;
    }
    template<typename E>
    bool EntityDelete(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return Delete(MY_CLIENT_ID, aKey);
    }
    template<typename E>
    bool EntityUpdateOrCreate(E& entity)
    {
        if (EntityExists(entity))
            return EntityUpdate(entity);
        return EntityCreate(entity);
    }
    template<typename E>
    bool EntityUpdate(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        ea::shared_ptr<StorageData> data = ea::make_shared<StorageData>();
        if (SetEntity<E>(entity, *data) == 0)
            return false;
        return Update(MY_CLIENT_ID, aKey, data);
    }
    template<typename E>
    bool EntityCreate(E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        ea::shared_ptr<StorageData> data = ea::make_shared<StorageData>();
        if (SetEntity<E>(entity, *data) == 0)
            return false;
        return Create(MY_CLIENT_ID, aKey, data);
    }
    template<typename E>
    bool EntityPreload(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return Preload(MY_CLIENT_ID, aKey);
    }
    template<typename E>
    bool EntityExists(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        ea::shared_ptr<StorageData> data = ea::make_shared<StorageData>();
        if (SetEntity<E>(entity, *data) == 0)
            return false;
        return Exists(MY_CLIENT_ID, aKey, data);
    }
    /// Flushes an entity and removes it from cache
    template<typename E>
    bool EntityInvalidate(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return Invalidate(MY_CLIENT_ID, aKey);
    }
    void Shutdown();
    void UnlockAll(uint32_t clientId);
    uint32_t flushInterval_;
    uint32_t cleanInterval_;
private:
    static constexpr uint32_t MY_CLIENT_ID = std::numeric_limits<uint32_t>::max();
    struct CacheItem
    {
        CacheFlags flags{ 0 };
        uint32_t locker{ 0 };
        ea::shared_ptr<StorageData> data;
    };
    sa::CallableTable<size_t, bool, StorageData&> exitsCallables_;
    sa::CallableTable<size_t, bool, CacheItem&> flushCallables_;
    sa::CallableTable<size_t, bool, const uuids::uuid&, StorageData&> loadCallables_;
    template<typename D, typename E>
    void AddEntityClass()
    {
        static constexpr size_t hash = sa::StringHash(E::KEY());
        // For each Entity there are 3 methods that we need:
        // 1. Does the Entity exist
        // 2. Write (also delete) the entity to the DB
        // 3. Load the Entity from the DB
        // Yes, I love templates too!
        exitsCallables_.Add(hash, [this](auto& data) -> bool
        {
            return ExistsInDB<D, E>(data);
        });
        flushCallables_.Add(hash, [this](CacheItem& data) -> bool
        {
            return FlushRecord<D, E>(data);
        });
        loadCallables_.Add(hash, [this](const auto& id, auto& data) -> bool
        {
            return LoadFromDB<D, E>(id, data);
        });
    }
    void InitEnitityClasses();

    /// Read UUID from data
    static uuids::uuid GetUuid(StorageData& data);

    bool EnoughSpace(size_t size);
    void CreateSpace(size_t size);
    void CacheData(const std::string& table, const uuids::uuid& id,
        ea::shared_ptr<StorageData> data,
        CacheFlags flags);
    bool RemoveData(uint32_t clientId, const IO::DataKey& key);
    void PreloadTask(IO::DataKey key);
    bool ExistsData(const IO::DataKey& key, StorageData& data);
    /// If the data is a player and it's in playerNames_ remove it from playerNames_
    void RemovePlayerFromCache(const IO::DataKey& key);

    void CleanCache();
    void CleanTask();
    void FlushCache();
    void FlushCacheTask();
    void ClearPrices();
    void ClearPricesTask();

    /// Loads Data from DB
    bool LoadData(const IO::DataKey& key, ea::shared_ptr<StorageData> data);
    template<typename D, typename E>
    bool LoadFromDB(const uuids::uuid& id, StorageData& data)
    {
        E e{};
        if (!id.nil())
            // An UUID is all we need to identify a record
            e.uuid = id.to_string();
        else
        {
            // If not, get it from the data
            if (!GetEntity<E>(data, e))
                return false;
        }
        if (D::Load(e))
            return (SetEntity<E>(e, data) != 0);
        return false;
    }

    /// Save data to DB or delete from DB.
    /// Depending on the data header calls CreateInDB(), SaveToDB() and/or DeleteFromDB()
    bool FlushData(uint32_t clientId, const IO::DataKey& key);
    template<typename D, typename E>
    bool FlushRecord(CacheItem& data)
    {
        bool succ = true;
        // These flags are not mutually exclusive, howerer creating it implies it is no longer modified
        if (!IsCreated(data.flags))
        {
            succ = CreateInDB<D, E>(*data.data);
            if (succ)
            {
                sa::bits::set(data.flags, CacheFlag::Created);
                sa::bits::un_set(data.flags, CacheFlag::Modified);
            }
        }
        else if (IsModified(data.flags))
        {
            succ = SaveToDB<D, E>(*data.data);
            if (succ)
                sa::bits::un_set(data.flags, CacheFlag::Modified);
        }
        if (IsDeleted(data.flags))
            succ = DeleteFromDB<D, E>(*data.data);

        return succ;
    }
    template<typename D, typename E>
    bool CreateInDB(StorageData& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
        {
            if (D::Create(e))
            {
                // Update data, id may have changed
                return (SetEntity<E>(e, data) != 0);
            }
        }
        return false;
    }
    template<typename D, typename E>
    bool SaveToDB(StorageData& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
            return D::Save(e);
        return false;
    }
    template<typename D, typename E>
    bool DeleteFromDB(StorageData& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
            return D::Delete(e);
        return false;
    }
    template<typename D, typename E>
    bool ExistsInDB(StorageData& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
            return D::Exists(e);
        return false;
    }

    template<typename E>
    static bool GetEntity(StorageData& data, E& e)
    {
        using InputAdapter = bitsery::InputBufferAdapter<StorageData>;
        InputAdapter ia(data.begin(), data.size());
        auto state = bitsery::quickDeserialization<InputAdapter, E>(ia, e);
        return state.first == bitsery::ReaderError::NoError;
    }
    template<typename E>
    static size_t SetEntity(const E& e, StorageData& buffer)
    {
        using OutputAdapter = bitsery::OutputBufferAdapter<StorageData>;
        return bitsery::quickSerialization<OutputAdapter, E>(buffer, e);
    }
    static bool IsUnlockedFor(uint32_t clientId, const CacheItem& item);

    bool readonly_;
    bool running_;
    size_t maxSize_;
    size_t currentSize_;

    ea::unordered_map<IO::DataKey, CacheItem, std::hash<IO::DataKey>> cache_;
    /// Name (Playername, Guildname etc.) -> Cache Key
    NameIndex namesCache_;
    CacheIndex index_;
};
