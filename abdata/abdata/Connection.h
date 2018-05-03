#pragma once

#include <stdint.h>
#include <vector>
#include "StorageProvider.h"
#include "Dispatcher.h"

class ConnectionManager;

enum OpCodes : uint8_t
{
    // Requests
    Create = 0,
    Update = 1,
    Read = 2,
    Delete = 3,
    // Invalidate a cache item. Flushes modified data. Next read will load it from the DB.
    // TODO: Check if needed.
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

class Connection : public std::enable_shared_from_this<Connection>
{
public:
	explicit Connection(asio::io_service& io_service, ConnectionManager& manager,
        StorageProvider& storage, size_t maxData, size_t maxKeySize_);
    asio::ip::tcp::socket& socket();
	void Start();
	void Stop();
private:
    template <typename Callable, typename... Args>
    void AddTask(Callable function, Args&&... args)
    {
        Asynch::Dispatcher::Instance.Add(
            Asynch::CreateTask(std::bind(function, this, std::forward<Args>(args)...))
        );
    }

    void StartCreateOperation();
    void StartUpdateDataOperation();
    void StartReadOperation();
    void StartDeleteOperation();
    void StartInvalidateOperation();
    void StartPreloadOperation();
    void StartExistsOperation();

    void HandleUpdateReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
    void HandleCreateReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
    void HandleReadReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
    void HandleWriteReqResponse(const asio::error_code& error);
    void HandleExistsReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
    void StartClientRequestedOp();
    void StartReadKey(uint16_t& keySize);
    void SendResponseAndStart(std::vector<asio::mutable_buffer>& resp, size_t size);
    void SendStatusAndRestart(ErrorCodes code, const std::string& message);

    static uint32_t toInt32(const std::vector<uint8_t>& intBytes, uint32_t start)
    {
        return (intBytes[start + 3] << 24) | (intBytes[start + 2] << 16) | (intBytes[start + 1] << 8) | intBytes[start];
    }
    static uint16_t toInt16(const std::vector<uint8_t>& intBytes, uint32_t start)
    {
        return  (intBytes[start + 1] << 8) | intBytes[start];
    }

    asio::ip::tcp::socket socket_;
    uint8_t opcode_;
    std::vector<uint8_t> key_;
    size_t maxDataSize_;
    size_t maxKeySize_;
    ConnectionManager& connectionManager_;
    StorageProvider& storageProvider_;
    std::shared_ptr<std::vector<uint8_t>> data_;
};

typedef std::shared_ptr<Connection> ConnectionPtr;