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

#include "stdafx.h"
#include "Server.h"
#include <iostream>
#include "ConnectionManager.h"
#include "Connection.h"
#include <abscommon/Scheduler.h>
#include <AB/CommonConfig.h>

Server::Server(asio::io_service& io_service, uint32_t ip,
    uint16_t port, size_t maxCacheSize, bool readonly,
    Net::IpList& whiteList) :
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
    acceptor_.async_accept(newConnection_->GetSocket(),
        std::bind(&Server::HandleAccept, this,
            std::placeholders::_1));
}

void Server::HandleAccept(const asio::error_code& error)
{
    if (!error)
    {
        const auto endp = newConnection_->GetSocket().remote_endpoint();
        if (IsIpAllowed(endp))
        {
            LOG_INFO << "Connection from " << endp.address() << ":" << endp.port() << std::endl;
            connectionManager_.Start(newConnection_);
        }
    }
    StartAccept();
}

bool Server::IsIpAllowed(const asio::ip::tcp::endpoint& ep)
{
    if (whiteList_.IsEmpty())
        return true;
    const uint32_t ip = ep.address().to_v4().to_uint();
    const bool result = whiteList_.Contains(ip);
    if (!result)
        LOG_WARNING << "Connection attempt from a not allowed IP " << Utils::ConvertIPToString(ip) << std::endl;
    return result;
}
