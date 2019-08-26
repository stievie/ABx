#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>
#include "EvictionStrategy.h"
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4310 4100)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#if defined(__GCC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(__GCC__)
#pragma GCC diagnostic pop
#endif
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#include "StringHash.h"
#include "DataKey.h"
#include <sa/CallableTable.h>
#include "StringHash.h"

// Clean cache every 10min
#define CLEAN_CACHE_MS (1000 * 60 * 10)
// Flush cache every minute
#define FLUSH_CACHE_MS (1000 * 60)

struct CacheFlags
{
    bool created;             // It exists in DB
    bool modified;
    bool deleted;
};

class StorageProvider
{
public:
    StorageProvider(size_t maxSize, bool readonly);

    bool Create(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data);
    bool Update(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data);
    bool Read(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data);
    bool Delete(const IO::DataKey& key);
    bool Invalidate(const IO::DataKey& key);
    bool Preload(const IO::DataKey& key);
    bool Exists(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data);
    bool Clear(const IO::DataKey& key);

    // Client compatible Methods
    template<typename E>
    bool EntityRead(E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>();
        SetEntity<E>(entity, *data.get());
        if (!Read(aKey, data))
            return false;
        if (GetEntity(*data.get(), entity))
            return true;
        return false;
    }
    template<typename E>
    bool EntityDelete(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return Delete(aKey);
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
        std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>();
        if (SetEntity<E>(entity, *data.get()) == 0)
            return false;
        return Update(aKey, data);
    }
    template<typename E>
    bool EntityCreate(E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>();
        if (SetEntity<E>(entity, *data.get()) == 0)
            return false;
        return Create(aKey, data);
    }
    template<typename E>
    bool EntityPreload(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return Preload(aKey);
    }
    template<typename E>
    bool EntityExists(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        std::shared_ptr<std::vector<uint8_t>> data = std::make_shared<std::vector<uint8_t>>();
        if (SetEntity<E>(entity, *data.get()) == 0)
            return false;
        return Exists(aKey, data);
    }
    /// Flushes an entity and removes it from cache
    template<typename E>
    bool EntityInvalidate(const E& entity)
    {
        const IO::DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return Invalidate(aKey);
    }
    /// Flush all
    void Shutdown();
    uint32_t flushInterval_;
    uint32_t cleanInterval_;
private:
    /// first = flags, second = data
    using CacheItem = std::pair<CacheFlags, std::shared_ptr<std::vector<uint8_t>>>;
    sa::CallableTable<size_t, bool, std::vector<uint8_t>&> exitsCallables_;
    sa::CallableTable<size_t, bool, CacheItem&> flushCallables_;
    sa::CallableTable<size_t, bool, const uuids::uuid&, std::vector<uint8_t>&> loadCallables_;
    template<typename D, typename E>
    void AddEntityClass()
    {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4307)
#endif
        constexpr size_t hash = Utils::StringHash(E::KEY());
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
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
    static uuids::uuid GetUuid(std::vector<uint8_t>& data);

    bool EnoughSpace(size_t size);
    void CreateSpace(size_t size);
    void CacheData(const std::string& table, const uuids::uuid& id,
        std::shared_ptr<std::vector<uint8_t>> data,
        bool modified, bool created);
    bool RemoveData(const IO::DataKey& key);
    void PreloadTask(IO::DataKey key);
    bool ExistsData(const IO::DataKey& key, std::vector<uint8_t>& data);
    /// If the data is a player and it's in playerNames_ remove it from playerNames_
    void RemovePlayerFromCache(const IO::DataKey& key);

    void CleanCache();
    void CleanTask();
    void FlushCache();
    void FlushCacheTask();

    /// Loads Data from DB
    bool LoadData(const IO::DataKey& key, std::shared_ptr<std::vector<uint8_t>> data);
    template<typename D, typename E>
    bool LoadFromDB(const uuids::uuid& id, std::vector<uint8_t>& data)
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
    /// So synchronize this item with the DB. Depending on the data header calls
    /// CreateInDB(), SaveToDB() and/or DeleteFromDB()
    bool FlushData(const IO::DataKey& key);
    template<typename D, typename E>
    bool FlushRecord(CacheItem& data)
    {
        bool succ = true;
        // These flags are not mutually exclusive, hoverer creating it implies it is no longer modified
        if (!data.first.created)
        {
            succ = CreateInDB<D, E>(*data.second.get());
            if (succ)
            {
                data.first.created = true;
                data.first.modified = false;
            }
        }
        else if (data.first.modified)
        {
            succ = SaveToDB<D, E>(*data.second.get());
            if (succ)
                data.first.modified = false;
        }
        if (data.first.deleted)
            succ = DeleteFromDB<D, E>(*data.second.get());

        return succ;
    }
    template<typename D, typename E>
    bool CreateInDB(std::vector<uint8_t>& data)
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
    bool SaveToDB(std::vector<uint8_t>& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
            return D::Save(e);
        return false;
    }
    template<typename D, typename E>
    bool DeleteFromDB(std::vector<uint8_t>& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
            return D::Delete(e);
        return false;
    }
    template<typename D, typename E>
    bool ExistsInDB(std::vector<uint8_t>& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
            return D::Exists(e);
        return false;
    }

    template<typename E>
    static bool GetEntity(std::vector<uint8_t>& data, E& e)
    {
        using InputAdapter = bitsery::InputBufferAdapter<std::vector<uint8_t>>;
        InputAdapter ia(data.begin(), data.size());
        auto state = bitsery::quickDeserialization<InputAdapter, E>(ia, e);
        return state.first == bitsery::ReaderError::NoError;
    }
    template<typename E>
    static size_t SetEntity(const E& e, std::vector<uint8_t>& buffer)
    {
        using Buffer = std::vector<uint8_t>;
        using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
        auto writtenSize = bitsery::quickSerialization<OutputAdapter, E>(buffer, e);
        return writtenSize;
    }

    bool readonly_;
    bool running_;
    size_t maxSize_;
    size_t currentSize_;

    std::unordered_map<IO::DataKey, CacheItem> cache_;
    std::mutex lock_;
    /// Player name -> Cache Key
    std::unordered_map<std::string, IO::DataKey> playerNames_;
    OldestInsertionEviction evictor_;
};

