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

struct CacheFlags
{
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
    static uuids::uuid GetUuid(std::shared_ptr<std::vector<uint8_t>> data);
    bool EnoughSpace(size_t size);
    void CreateSpace(size_t size);
    void CacheData(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data, bool modified);
    bool RemoveData(const std::string& key);

    /// Create data in DB. Returns id (primary key)
    bool CreateData(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
    template<typename D, typename E>
    bool CreateInDB(std::vector<uint8_t>& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
        {
            if (D::Create(e))
            {
                // Update data, id may have changed
                if (SetEntity<E>(e, data) != 0)
                    return true;
                return false;
            }
        }
        return false;
    }
    /// Loads Data from DB
    bool LoadData(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
    template<typename D, typename E>
    bool LoadFromDB(const uuids::uuid& id, std::vector<uint8_t>& data)
    {
        E e{};
        if (!id.nil())
            e.uuid = id.to_string();
        else
        {
            GetEntity<E>(data, e);
        }
        if (D::Load(e))
        {
            if (SetEntity<E>(e, data) != 0)
                return true;
            return false;
        }
        return false;
    }

    /// Save data to DB or delete from DB.
    /// So synchronize this item with the DB.
    void FlushData(const std::vector<uint8_t>& key);
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
    std::unordered_map<std::string, CacheItem> cache_;
    size_t currentSize_;
    size_t maxSize_;
    std::shared_ptr<EvictionStrategy> evictor_;
};

