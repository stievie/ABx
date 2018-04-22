#pragma once

#pragma warning(push)
#pragma warning(disable: 4310 4100)
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>
#pragma warning(pop)

enum OpCodes : uint8_t
{
    // Requests
    Create = 0,
    Update = 1,
    Read = 2,
    Delete = 3,
    // Invalidate a cache item. Does NOT flush modified data. Next read will load it from the DB.
    // TODO: Check if needed.
    Invalidate = 4,
    // Responses
    Status,
    Data
};

enum ErrorCodes : uint8_t
{
    Ok,
    NoSuchKey,
    KeyTooBig,
    DataTooBig,
    OtherErrors
};

class DataClient
{
public:
    explicit DataClient(asio::io_service& io_service);
    ~DataClient();

    void Connect(const std::string& host, uint16_t port);

    template<typename E>
    bool Read(E& entity)
    {
        std::vector<uint8_t> aKey = EncodeKey(E::KEY(), uuids::uuid(entity.uuid));
        std::vector<uint8_t> data;
        SetEntity<E>(entity, data);
        if (!ReadData(aKey, data))
            return false;
        if (GetEntity(data, entity))
            return true;
        return false;
    }
    /// Delete an entity. This entity must be in cache, if not, use Read first.
    template<typename E>
    bool Delete(const E& entity)
    {
        std::vector<uint8_t> aKey = EncodeKey(E::KEY(), uuids::uuid(entity.uuid));
        return DeleteData(aKey);
    }
    template<typename E>
    bool Update(const E& entity)
    {
        std::vector<uint8_t> aKey = EncodeKey(E::KEY(), uuids::uuid(entity.uuid));
        std::vector<uint8_t> data;
        if (SetEntity<E>(entity, data) == 0)
            return false;
        return UpdateData(aKey, data);
    }
    template<typename E>
    bool Create(E& entity)
    {
        std::vector<uint8_t> aKey = EncodeKey(E::KEY(), uuids::uuid(entity.uuid));
        std::vector<uint8_t> data;
        if (SetEntity<E>(entity, data) == 0)
            return false;
        return CreateData(aKey, data);
    }
    bool IsConnected() const
    {
        return connected_;
    }
private:
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

    static bool DecodeKey(const std::vector<uint8_t>& key, std::string& table, uuids::uuid& id)
    {
        // key = <tablename><guid>
        if (key.size() <= uuids::uuid::state_size)
            return false;
        table.assign(key.begin(), key.end() - uuids::uuid::state_size);
        id = uuids::uuid(key.end() - uuids::uuid::state_size, key.end());
        return true;
    }
    static std::vector<uint8_t> EncodeKey(const std::string& table, const uuids::uuid& id)
    {
        std::vector<uint8_t> result(table.begin(), table.end());
        result.insert(result.end(), id.begin(), id.end());
        return result;
    }
    static uint32_t ToInt32(const std::vector<uint8_t>& intBytes, uint32_t start)
    {
        return (intBytes[start + 3] << 24) | (intBytes[start + 2] << 16) | (intBytes[start + 1] << 8) | intBytes[start];
    }
    static uint16_t ToInt16(const std::vector<uint8_t>& intBytes, uint32_t start)
    {
        return  (intBytes[start + 1] << 8) | intBytes[start];
    }

    bool ReadData(const std::vector<uint8_t>& key, std::vector<uint8_t>& data);
    bool DeleteData(const std::vector<uint8_t>& key);
    bool UpdateData(const std::vector<uint8_t>& key, std::vector<uint8_t>& data);
    bool CreateData(const std::vector<uint8_t>& key, std::vector<uint8_t>& data);
    void InternalConnect();
    bool TryConnect(bool force);
    bool CheckConnection();

    std::string host_;
    uint16_t port_;
    asio::ip::tcp::socket socket_;
    asio::ip::tcp::resolver resolver_;
    bool connected_;
};

