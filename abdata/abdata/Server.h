#pragma once

#include "Connection.h"
#include "ConnectionManager.h"

class Server {

public:
	Server(asio::io_service& io_service, uint32_t ip,
        uint16_t port, size_t maxCacheSize, bool readonly);
    ~Server();
    void Shutdown();

    StorageProvider* GetStorageProvider()
    {
        return &storageProvider_;
    }
private:
    void LogRotateTask();
	void StartAccept();
	void HandleAccept(const asio::error_code& error);
    bool running_;
	asio::io_service& io_service_;
	asio::ip::tcp::acceptor acceptor_;
	ConnectionPtr newConnection_;
	ConnectionManager connectionManager_;
	StorageProvider storageProvider_;
	size_t maxDataSize_;
	size_t maxKeySize_;
};
