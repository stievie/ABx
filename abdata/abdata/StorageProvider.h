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
#include "StringHash.h"

#pragma warning(push)
#pragma warning(disable: 4307)
static constexpr size_t TABLE_ACCOUNTS = Utils::StringHash("accounts");
static constexpr size_t TABLE_CHARACTERS = Utils::StringHash("players");
#pragma warning(pop)

class StorageProvider
{
public:
	StorageProvider(size_t maxSize);
	void Save(const std::vector<uint8_t>& key, std::shared_ptr<std::vector<uint8_t>> data);
	std::shared_ptr<std::vector<uint8_t>> Get(const std::vector<uint8_t>& key);
	bool Delete(const std::vector<uint8_t>& key);
    void Shutdown();
    static bool DecodeKey(const std::vector<uint8_t>& key, std::string& table, uint32_t& id);
    static std::vector<uint8_t> EncodeKey(const std::string& table, uint32_t id);
private:
	bool EnoughSpace(size_t size);
	void CreateSpace(size_t size);
	bool RemoveData(const std::string& key);
    /// Loads Data from DB
    std::shared_ptr<std::vector<uint8_t>> LoadData(const std::vector<uint8_t>& key);
    template<typename D, typename E>
    std::shared_ptr<std::vector<uint8_t>> LoadFromDB(uint32_t id)
    {
        E e;
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

	std::unordered_map<std::string, std::shared_ptr<std::vector<uint8_t>>> cache_;
	size_t currentSize_;
	size_t maxSize_;
	std::shared_ptr<EvictionStrategy> evictor_;
};

