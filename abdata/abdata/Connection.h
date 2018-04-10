#pragma once

#include <stdint.h>
#include <vector>
#include "StorageProvider.h"

class ConnectionManager;

enum OpCodes
{
	Set,
    Get,
    Delete,
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

class Connection:public std::enable_shared_from_this<Connection> {
public:
	explicit Connection(asio::io_service& io_service, ConnectionManager& manager,
        StorageProvider& storage, size_t maxData, size_t maxKeySize_);
    asio::ip::tcp::socket& socket();
	void Start();
	void Stop();
private:
	void HandleReadRawData(const asio::error_code& error, size_t bytes_transferred, size_t expected);
	void HandleWriteReqResponse(const asio::error_code& error);
	void StartClientRequestedOp();
	void StartReadKey(uint16_t& keySize);
	void StartSetDataOperation();
	void StartGetOperation();
	void StartDeleteOperation();
	void SendResponseAndStart(std::vector<asio::mutable_buffer>& resp, size_t size);
	void SendStatusAndRestart(ErrorCodes code, std::string message);

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