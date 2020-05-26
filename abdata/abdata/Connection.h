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

#include <stdint.h>
#include <vector>
#include "StorageProvider.h"
#include <abscommon/Dispatcher.h>
#include <abscommon/DataKey.h>
#include <abscommon/Subsystems.h>
#include <abscommon/DataCodes.h>
#include <asio.hpp>
#include <eastl.hpp>
#include <mutex>

class ConnectionManager;

class Connection : public ea::enable_shared_from_this<Connection>
{
public:
    explicit Connection(asio::io_service& io_service, ConnectionManager& manager,
        StorageProvider& storage, size_t maxData, uint16_t maxKeySize_);
    asio::ip::tcp::socket& GetSocket();
    void Start();
    void Stop();
private:
    template <typename Callable, typename... Args>
    void AddTask(Callable&& function, Args&&... args)
    {
        GetSubsystem<Asynch::Dispatcher>()->Add(
            Asynch::CreateTask(std::bind(std::move(function), shared_from_this(), std::forward<Args>(args)...))
        );
    }

    void StartCreateOperation();
    void StartUpdateDataOperation();
    void StartReadOperation();
    void StartDeleteOperation();
    void StartInvalidateOperation();
    void StartPreloadOperation();
    void StartExistsOperation();
    void StartClearOperation();

    void HandleUpdateReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
    void HandleCreateReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
    void HandleReadReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
    void HandleExistsReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
    void HandleWriteReqResponse(const asio::error_code& error);

    void StartClientRequestedOp();
    void StartReadKey(uint16_t keySize);
    void SendResponseAndStart(std::vector<asio::mutable_buffer>& resp, size_t size);
    void SendStatusAndRestart(IO::ErrorCodes code, const std::string& message);

    static inline uint32_t ToInt32(const StorageData& intBytes, size_t start)
    {
        return (intBytes[start + 3] << 24) | (intBytes[start + 2] << 16) | (intBytes[start + 1] << 8) | intBytes[start];
    }
    static inline uint16_t ToInt16(const StorageData& intBytes, size_t start)
    {
        return  (intBytes[start + 1] << 8) | intBytes[start];
    }

    size_t maxDataSize_;
    uint16_t maxKeySize_;
    asio::ip::tcp::socket socket_;
    ConnectionManager& connectionManager_;
    StorageProvider& storageProvider_;
    IO::OpCodes opcode_;
    std::mutex lock_;

    IO::DataKey key_;
    ea::shared_ptr<StorageData> data_;
};
