#pragma once

#include "Connection.h"
#include "ConnectionManager.h"
#include "IpList.h"

class Server {

public:
	Server(asio::io_service& io_service, uint32_t ip,
        uint16_t port, size_t maxCacheSize, bool readonly,
        Net::IpList& whiteList);
    ~Server();
    void Shutdown();

    StorageProvider& GetStorageProvider()
    {
        return storageProvider_;
    }
private:
	void StartAccept();
	void HandleAccept(const asio::error_code& error);
    bool IsIpAllowed(const asio::ip::tcp::endpoint& ep);
    bool running_;
	asio::io_service& io_service_;
	asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<Connection> newConnection_;
	ConnectionManager connectionManager_;
	StorageProvider storageProvider_;
    Net::IpList& whiteList_;
	size_t maxDataSize_;
    uint16_t maxKeySize_;
};
