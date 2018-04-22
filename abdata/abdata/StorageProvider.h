#pragma once
#include <unordered_map>
#include <vector>
#include "EvictionStrategy.h"
#pragma warning(push)
#pragma warning(disable: 4310 4100)
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>
#pragma warning(pop)

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

    bool Create(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
    bool Update(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
    bool Read(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
    bool Delete(const std::vector<uint8_t>& key);
    bool Invalidate(const std::vector<uint8_t>& key);

    /// Flush all
    void Shutdown();
private:
    /// first = dirty, second = data
    using CacheItem = std::pair<CacheFlags, std::shared_ptr<std::vector<uint8_t>>>;

    static bool DecodeKey(const std::vector<uint8_t>& key, std::string& table, uuids::uuid& id);
    static std::vector<uint8_t> EncodeKey(const std::string& table, const uuids::uuid& id);
    /// Read UUID from data
    static uuids::uuid GetUuid(std::shared_ptr<std::vector<uint8_t>> data);

    bool EnoughSpace(size_t size);
    void CreateSpace(size_t size);
    void CacheData(const std::vector<uint8_t>& key,
        std::shared_ptr<std::vector<uint8_t>> data,
        bool modified, bool created);
    bool RemoveData(const std::string& key);

    void CleanCache();
    void CleanCacheTask();
    void FlushCache();
    void FlushCacheTask();

    /// Loads Data from DB
    bool LoadData(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
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
    bool FlushData(const std::vector<uint8_t>& key);
    template<typename D, typename E, typename I>
    bool FlushRecord(I& data)
    {
        bool succ = true;
        // These flags are not mutually exclusive, hoverer creating it implies it is no longer modified
        if (!(*data).second.first.created)
        {
            succ = CreateInDB<D, E>(*(*data).second.second.get());
            if (succ)
            {
                (*data).second.first.created = true;
                (*data).second.first.modified = false;
            }
        }
        else if ((*data).second.first.modified)
        {
            succ = SaveToDB<D, E>(*(*data).second.second.get());
            if (succ)
                (*data).second.first.modified = false;
        }
        if ((*data).second.first.deleted)
            succ = DeleteFromDB<D, E>(*(*data).second.second.get());

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
    std::mutex lock_;
    std::unordered_map<std::string, CacheItem> cache_;
    size_t currentSize_;
    size_t maxSize_;
    std::unique_ptr<EvictionStrategy> evictor_;
};

