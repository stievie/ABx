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

enum ErrorCodes
{
	Ok,NoSuchKey,KeyTooBig,DataTooBig,OtherErrors
};

class Connection:public std::enable_shared_from_this<Connection> {
public:
	explicit Connection(asio::io_service& io_service, ConnectionManager& manager,
        StorageProvider& storage,int maxData,int maxKeySize_);
    asio::ip::tcp::socket& socket();
	void start();
	void stop();
private:
	/*void handleReadOpcode(const boost::system::error_code& error,unsigned int bytes_transferred);

	void handleReadKey(const boost::system::error_code& error, boost::uint16_t byteTransferred, boost::uint16_t expectedkeySize);

	void handleReadRawDataHeader(const boost::system::error_code& error, unsigned int bytes_transferred);
*/
	void handleReadRawData(const asio::error_code& error, uint32_t bytes_transferred, uint32_t expected);

	void handleWriteReqResponse(const asio::error_code& error);

	void startClientRequestedOp();

	void startReadKey(uint16_t& keySize);

	void startSetDataOperation();

	void startGetOperation();

	void startDeleteOperation();

	void sendResponseAndStart(std::vector<asio::mutable_buffer>& resp, uint32_t size);

	uint32_t toInt32(const std::vector<uint8_t>& intBytes, uint32_t start);

	uint16_t toInt16(const std::vector<uint8_t>& intBytes, uint32_t start);

	void sendStatusAndRestart(ErrorCodes code, std::string message);

	void retriveAndSendData(uint16_t key);

	void deleteData(uint16_t key);


    asio::ip::tcp::socket socket_;

	uint8_t opcode_;
	std::vector<uint8_t> key_;
	uint32_t maxDataSize_;
	uint32_t maxKeySize_;
	ConnectionManager& connectionManager_;
	StorageProvider& storageProvider_;
	std::shared_ptr<std::vector<uint8_t>> data_;
};

typedef std::shared_ptr<Connection> ConnectionPtr;