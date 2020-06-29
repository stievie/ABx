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

#include <sa/Compiler.h>

PRAGMA_WARNING_PUSH
    PRAGMA_WARNING_DISABLE_MSVC(4310 4100)
#   include <bitsery/bitsery.h>
#   include <bitsery/adapter/buffer.h>
#   include <bitsery/traits/vector.h>
#   include <bitsery/traits/string.h>
PRAGMA_WARNING_POP

#include <uuid.h>
#include "DataKey.h"
#include "DataCodes.h"
#include <mutex>
#include <asio.hpp>

namespace IO {

using DataBuff = std::vector<uint8_t>;

class DataClient
{
private:
    std::mutex lock_;
public:
    explicit DataClient(asio::io_service& io_service);
    ~DataClient();

    void Connect(const std::string& host, uint16_t port);

    // Lock this entity so it can only be modified by this client, i.e. make it read-only for all other clients.
    template<typename E>
    bool Lock(const E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return LockData(aKey);
    }
    // Unlock: Make it editable for all.
    template<typename E>
    bool Unlock(const E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        return UnlockData(aKey);
    }
    template<typename E>
    bool Read(E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        DataBuff data;
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
    bool DeleteIfExists(E& entity)
    {
        // Can not use Exists(), because we can only delete records that are in cache,
        // i.e. previously read.
        if (Read(entity))
            return Delete(entity);
        // Doesn't exists -> success
        return true;
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
        DataBuff data;
        if (SetEntity<E>(entity, data) == 0)
            return false;
        return UpdateData(aKey, data);
    }
    template<typename E>
    bool Create(E& entity)
    {
        const DataKey aKey(E::KEY(), uuids::uuid(entity.uuid));
        DataBuff data;
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
        DataBuff data;
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
    /// Clears all cache
    bool Clear();
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
    /// Unserialize Entitiy
    /// @param[in] data Input data
    /// @param[out] Resulting Entity
    /// @return true on success
    template<typename E>
    static bool GetEntity(DataBuff& data, E& e)
    {
        using InputAdapter = bitsery::InputBufferAdapter<DataBuff>;
        auto state = bitsery::quickDeserialization<InputAdapter, E>({ data.begin(), data.size() }, e);
        return state.first == bitsery::ReaderError::NoError;
    }
    /// Serialize Entity.
    /// @param[in] Input Entity
    /// @param[out] Serialized data
    /// @return Size needed
    template<typename E>
    static size_t SetEntity(const E& e, DataBuff& buffer)
    {
        using OutputAdapter = bitsery::OutputBufferAdapter<DataBuff>;
        auto writtenSize = bitsery::quickSerialization<OutputAdapter, E>(buffer, e);
        return writtenSize;
    }

    static uint32_t ToInt32(const DataBuff& intBytes, uint32_t start)
    {
        return (intBytes[static_cast<size_t>(start) + 3] << 24) |
            (intBytes[static_cast<size_t>(start) + 2] << 16) |
            (intBytes[static_cast<size_t>(start) + 1] << 8) |
            intBytes[static_cast<size_t>(start)];
    }
    static uint16_t ToInt16(const DataBuff& intBytes, uint32_t start)
    {
        return (intBytes[static_cast<size_t>(start) + 1] << 8) |
            intBytes[static_cast<size_t>(start)];
    }

    bool MakeRequest(OpCodes opCode, const DataKey& key, DataBuff& data);
    bool MakeRequestNoData(OpCodes opCode, const DataKey& key);
    bool LockData(const DataKey& key);
    bool UnlockData(const DataKey& key);
    bool ReadData(const DataKey& key, DataBuff& data);
    bool DeleteData(const DataKey& key);
    bool ExistsData(const DataKey& key, DataBuff& data);
    bool UpdateData(const DataKey& key, DataBuff& data);
    bool CreateData(const DataKey& key, DataBuff& data);
    bool PreloadData(const DataKey& key);
    bool InvalidateData(const DataKey& key);
    void InternalConnect();
    /// Try connect to server.
    /// @param[in] force If force is true it disconnects first.
    /// @return true on success.
    bool TryConnect(bool force, unsigned numTries = 10);
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
    uint16_t port_{ 0 };
    asio::ip::tcp::socket socket_;
    asio::ip::tcp::resolver resolver_;
    bool connected_{ false };
};

// RAII Entity locker
template<typename E>
class EntityLocker
{
private:
    const E& entity_;
    DataClient& client_;
    bool locked_;
public:
    EntityLocker(DataClient& client, const E& entity) :
        entity_(entity),
        client_(client),
        locked_(false)
    { }
    ~EntityLocker()
    {
        if (locked_)
            client_.Unlock(entity_);
    }
    bool Lock()
    {
        locked_ = client_.Lock(entity_);
        return locked_;
    }
};

template <typename E> EntityLocker(DataClient&, const E&) -> EntityLocker<E>;

}
