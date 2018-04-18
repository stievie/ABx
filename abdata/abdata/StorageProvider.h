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

class StorageProvider
{
public:
    StorageProvider(size_t maxSize, bool readonly);

    uint32_t Create(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
    bool Update(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
    std::shared_ptr<std::vector<uint8_t>> Read(const std::vector<uint8_t>& key);
    bool Delete(const std::vector<uint8_t>& key);

    /// Flush all
    void Shutdown();
private:
    /// first = dirty, second = data
    using CacheItem = std::pair<bool, std::shared_ptr<std::vector<uint8_t>>>;

    static bool DecodeKey(const std::vector<uint8_t>& key, std::string& table, uint32_t& id);
    static std::vector<uint8_t> EncodeKey(const std::string& table, uint32_t id);
    bool EnoughSpace(size_t size);
    void CreateSpace(size_t size);
    void CacheData(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data, bool modified);
    bool RemoveData(const std::string& key);

    /// Create data in DB. Returns id (primary key)
    uint32_t CreateData(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
    template<typename D, typename E>
    uint32_t CreateInDB(std::vector<uint8_t>& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
        {
            uint32_t id = D::Create(e);
            if (id != 0)
            {
                // Update data, id changed
                auto d = SetEntity<E>(e);
                data.assign(d->begin(), d->end());
            }
            return id;
        }
        return 0;
    }
    /// Loads Data from DB
    std::shared_ptr<std::vector<uint8_t>> LoadData(const std::vector<uint8_t>& key);
    template<typename D, typename E>
    std::shared_ptr<std::vector<uint8_t>> LoadFromDB(uint32_t id)
    {
        E e{};
        e.id = id;
        if (D::Load(e))
            return SetEntity<E>(e);
        return std::shared_ptr<std::vector<uint8_t>>();
    }

    /// Save data to DB
    void FlushData(const std::vector<uint8_t>& key);
    template<typename D, typename E>
    bool SaveToDB(std::vector<uint8_t>& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
            return D::Save(e);
        return false;
    }

    /// Delete data from DB
    void DeleteData(const std::vector<uint8_t>& key);
    template<typename D, typename E>
    bool DeleteFromDB(std::vector<uint8_t>& data)
    {
        E e{};
        if (GetEntity<E>(data, e))
            return D::Delete(e);
        return false;
    }

    template<typename E>
    bool GetEntity(std::vector<uint8_t>& data, E& e)
    {
        using Buffer = std::vector<uint8_t>;
        using InputAdapter = bitsery::InputBufferAdapter<Buffer>;
        auto state = bitsery::quickDeserialization<InputAdapter, E>({ data.begin(), data.end() }, e);
        return state.first == bitsery::ReaderError::NoError && state.second;
    }
    template<typename E>
    std::shared_ptr<std::vector<uint8_t>> SetEntity(const E& e)
    {
        using Buffer = std::vector<uint8_t>;
        using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
        Buffer buffer;
        /*auto writtenSize =*/ bitsery::quickSerialization<OutputAdapter, E>(buffer, e);
        return std::shared_ptr<std::vector<uint8_t>>(&buffer);
    }

    bool readonly_;
    std::unordered_map<std::string, CacheItem> cache_;
    size_t currentSize_;
    size_t maxSize_;
    std::shared_ptr<EvictionStrategy> evictor_;
};

