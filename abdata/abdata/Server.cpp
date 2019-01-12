#include "stdafx.h"
#include "Server.h"
#include <iostream>
#include "ConnectionManager.h"
#include "Connection.h"
#include "Scheduler.h"
#include <AB/CommonConfig.h>

Server::Server(asio::io_service& io_service, uint32_t ip,
    uint16_t port, size_t maxCacheSize, bool readonly,
    Net::IpList& whiteList) :
    running_(false),
    io_service_(io_service),
    acceptor_(
        io_service,
        asio::ip::tcp::endpoint(asio::ip::address(asio::ip::address_v4(ip)), port)
    ),
    storageProvider_(maxCacheSize, readonly),
    whiteList_(whiteList),
    maxDataSize_(MAX_DATA_SIZE),
    maxKeySize_(MAX_KEY_SIZE)
{
#ifdef TCP_OPTION_NODELAY
    acceptor_.set_option(asio::ip::tcp::no_delay(true));
#endif
    StartAccept();
    running_ = true;
}

Server::~Server()
{
    if (running_)
        Shutdown();
}

void Server::Shutdown()
{
    connectionManager_.StopAll();
    storageProvider_.Shutdown();
    io_service_.stop();
    running_ = false;
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
        if (IsIpAllowed(endp))
        {
            LOG_INFO << "Connection from " << endp.address() << ":" << endp.port() << std::endl;
            connectionManager_.Start(newConnection_);
        }
        else
            LOG_ERROR << "Connection from " << endp.address() << " not allowed" << std::endl;
    }
    StartAccept();
}

bool Server::IsIpAllowed(const asio::ip::tcp::endpoint& ep)
{
    if (whiteList_.IsEmpty())
        return true;
    return whiteList_.Contains(ep.address().to_v4().to_uint());
}
