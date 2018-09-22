#pragma once

#pragma warning(push)
#pragma warning(disable: 4310 4100)
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>
#pragma warning(pop)
#include <uuid.h>
#include "DataKey.h"

namespace IO {

enum OpCodes : uint8_t
{
    // Requests
    Create = 0,
    Update = 1,
    Read = 2,
    Delete = 3,
    // Invalidate a cache item. Does NOT flush modified data. Next read will load it from the DB.
    Invalidate = 4,
    Preload = 5,
    Exists = 6,
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
    OtherErrors,
    NotExists
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
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
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
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return DeleteData(aKey);
    }
    template<typename E>
    bool UpdateOrCreate(E& entity)
    {
        if (Exists(entity))
            return Update(entity);
        return Create(entity);
    }
    template<typename E>
    bool Update(const E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        std::vector<uint8_t> data;
        if (SetEntity<E>(entity, data) == 0)
            return false;
        return UpdateData(aKey, data);
    }
    template<typename E>
    bool Create(E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        std::vector<uint8_t> data;
        if (SetEntity<E>(entity, data) == 0)
            return false;
        return CreateData(aKey, data);
    }
    template<typename E>
    bool Preload(const E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return PreloadData(aKey);
    }
    template<typename E>
    bool Exists(const E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        std::vector<uint8_t> data;
        if (SetEntity<E>(entity, data) == 0)
            return false;
        return ExistsData(aKey, data);
    }
    /// Flushes an entity and removes it from cache
    template<typename E>
    bool Invalidate(const E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return InvalidateData(aKey);
    }
    bool IsConnected() const
    {
        return connected_;
    }
    const std::string& GetHost() const
    {
        return host_;
    }
    uint16_t GetPort() const
    {
        return port_;
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

/*    static bool DecodeKey(const std::vector<uint8_t>& key, std::string& table, uuids::uuid& id)
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
    }*/
    static uint32_t ToInt32(const std::vector<uint8_t>& intBytes, uint32_t start)
    {
        return (intBytes[start + 3] << 24) | (intBytes[start + 2] << 16) | (intBytes[start + 1] << 8) | intBytes[start];
    }
    static uint16_t ToInt16(const std::vector<uint8_t>& intBytes, uint32_t start)
    {
        return  (intBytes[start + 1] << 8) | intBytes[start];
    }

    bool MakeRequest(OpCodes opCode, const DataKey& key, std::vector<uint8_t>& data);
    bool MakeRequestNoData(OpCodes opCode, const DataKey& key);
    bool ReadData(const DataKey& key, std::vector<uint8_t>& data);
    bool DeleteData(const DataKey& key);
    bool ExistsData(const DataKey& key, std::vector<uint8_t>& data);
    bool UpdateData(const DataKey& key, std::vector<uint8_t>& data);
    bool CreateData(const DataKey& key, std::vector<uint8_t>& data);
    bool PreloadData(const DataKey& key);
    bool InvalidateData(const DataKey& key);
    void InternalConnect();
    bool TryConnect(bool force);
    /// Try to send some data. If not connected tries to reconnect.
    template<typename B>
    bool TryWrite(const B& buffer)
    {
        asio::error_code ec;
        socket_.send(buffer, 0, ec);
        if (ec)
        {
            if (!TryConnect(true))
                return false;
            socket_.send(buffer, 0, ec);
        }
        return !(bool)ec;
    }

    std::string host_;
    uint16_t port_;
    asio::ip::tcp::socket socket_;
    asio::ip::tcp::resolver resolver_;
    bool connected_;
};

}
