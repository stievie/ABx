#pragma once

#include <asio.hpp>
#include "Connection.h"
#include "ConnectionManager.h"

class Server {

public:
	Server(asio::io_service& io_service, uint32_t port, int maxCacheSize);


private:
	void startAccept();
	void handleAccept(const asio::error_code& error);
	asio::io_service& io_service_;
	asio::ip::tcp::acceptor acceptor_;
	ConnectionPtr newConnection_;
	ConnectionManager connectionManager_;
	StorageProvider storageProvider_;
	int maxDataSize_;
	int maxKeySize_;

};
