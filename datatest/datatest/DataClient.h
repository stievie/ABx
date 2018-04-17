#pragma once

#pragma warning(push)
#pragma warning(disable: 4310 4100)
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>
#pragma warning(pop)

enum OpCodes
{
    Create = 0,
    Update = 1,
    Read = 2,
    Delete = 3,
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
    bool Read(uint32_t id, E& entity)
    {
        std::vector<uint8_t> aKey = EncodeKey(E::KEY(), id);
        std::string sKey(aKey.begin(), aKey.end());
        std::shared_ptr<std::vector<uint8_t>> data = ReadData(sKey);
        if (!data)
            return false;
        if (GetEntity(*data.get(), entity))
            return true;
        return false;
    }
    template<typename E>
    bool Delete(const E& entity)
    {
        std::vector<uint8_t> aKey = EncodeKey(E::KEY(), entity.id);
        std::string sKey(aKey.begin(), aKey.end());
        return DeleteData(sKey);
    }
    template<typename E>
    bool Update(const E& entity)
    {
        std::vector<uint8_t> aKey = EncodeKey(E::KEY(), entity.id);
        std::string sKey(aKey.begin(), aKey.end());
        auto data = SetEntity<E>(entity);
        return UpdateData(sKey, data);
    }
    template<typename E>
    bool Create(uint32_t& id, E& entity)
    {
        std::vector<uint8_t> aKey = EncodeKey(E::KEY(), 0);
        std::string sKey(aKey.begin(), aKey.end());
        auto data = SetEntity<E>(entity);
        id = CreateData(sKey, data);
        return id != 0;
    }
private:
    template<typename E>
    static bool GetEntity(std::vector<uint8_t>& data, E& e)
    {
        using Buffer = std::vector<uint8_t>;
        using InputAdapter = bitsery::InputBufferAdapter<Buffer>;
        auto state = bitsery::quickDeserialization<InputAdapter, E>({ data.begin(), data.end() }, e);
        return state.first == bitsery::ReaderError::NoError && state.second;
    }
    template<typename E>
    static std::shared_ptr<std::vector<uint8_t>> SetEntity(const E& e)
    {
        using Buffer = std::vector<uint8_t>;
        using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
        Buffer buffer;
        /*auto writtenSize =*/ bitsery::quickSerialization<OutputAdapter, E>(buffer, e);
        return std::shared_ptr<std::vector<uint8_t>>(&buffer);
    }

    static bool DecodeKey(const std::vector<uint8_t>& key, std::string& table, uint32_t& id)
    {
        // key = <tablename><id>
        if (key.size() <= sizeof(uint32_t))
            return false;
        table.assign(key.begin(), key.end() - sizeof(uint32_t));
        size_t start = key.size() - sizeof(uint32_t);
        id = (key[start + 3] << 24) | (key[start + 2] << 16) | (key[start + 1] << 8) | key[start];
        return false;
    }
    static std::vector<uint8_t> EncodeKey(const std::string& table, uint32_t id)
    {
        std::vector<uint8_t> result(table.begin(), table.end());
        result.push_back((uint8_t)id);
        result.push_back((uint8_t)(id >> 8));
        result.push_back((uint8_t)(id >> 16));
        result.push_back((uint8_t)(id >> 24));
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

    std::shared_ptr<std::vector<uint8_t>> ReadData(const std::string& key);
    bool DeleteData(const std::string& key);
    bool UpdateData(const std::string& key, std::shared_ptr<std::vector<uint8_t>> data);
    uint32_t CreateData(const std::string& key, std::shared_ptr<std::vector<uint8_t>> data);
    void InternalConnect();

    std::string host_;
    uint16_t port_;
    asio::ip::tcp::socket socket_;
    asio::ip::tcp::resolver resolver_;
};

