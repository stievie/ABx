#include "stdafx.h"
#include "Server.h"
#include <iostream>
#include "ConnectionManager.h"
#include "Connection.h"
#include "Logger.h"

Server::Server(asio::io_service& io_service,
    uint16_t port, size_t maxCacheSize, bool readonly) :
    io_service_(io_service),
    acceptor_(
        io_service,
        asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)
    ),
    runnig_(false),
    storageProvider_(maxCacheSize, readonly),
	maxDataSize_(1048576),
    maxKeySize_(256)
{
	StartAccept();
    runnig_ = true;
}

Server::~Server()
{
    if (runnig_)
        Shutdown();
}

void Server::Shutdown()
{
    connectionManager_.StopAll();
    storageProvider_.Shutdown();
    io_service_.stop();
    runnig_ = false;
}

void Server::StartAccept()
{
	newConnection_.reset(new Connection(io_service_, connectionManager_, storageProvider_, maxDataSize_, maxKeySize_));
	acceptor_.async_accept(newConnection_->socket(),
        std::bind(&Server::HandleAccept, this,
            std::placeholders::_1));
}

void Server::HandleAccept(const asio::error_code& error)
{
	if (!error)
	{
		auto endp = newConnection_.get()->socket().remote_endpoint();
        LOG_INFO << "Connection from " << endp.address() << ":" << endp.port() << std::endl;
		connectionManager_.Start(newConnection_);
	}
	StartAccept();
}
