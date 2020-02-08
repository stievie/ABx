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
#include "ServerConnection.h"
#include "MessageBuffer.h"

namespace IPC {

Server::Server(asio::io_service& ioService, const asio::ip::tcp::endpoint& endpoint) :
    ioService_(ioService),
    acceptor_(ioService, endpoint)
{
    StartAccept();
}

void Server::RemoveConnection(std::shared_ptr<ServerConnection> conn)
{
    clients_.erase(conn);
}

void Server::AddConnection(std::shared_ptr<ServerConnection> conn)
{
    clients_.emplace(conn);
}

void Server::InternalSend(const MessageBuffer& msg)
{
    std::for_each(clients_.begin(), clients_.end(),
        std::bind(&ServerConnection::Send, std::placeholders::_1, msg));
}

void Server::HandleMessage(const MessageBuffer& msg)
{
    if (handlers_.Exists(msg.type_))
        handlers_.Call(msg.type_, msg);
}

void Server::StartAccept()
{
    std::shared_ptr<ServerConnection> connection = std::make_shared<ServerConnection>(ioService_, *this);
    acceptor_.async_accept(connection->GetSocket(),
        std::bind(&Server::HandleAccept, this, connection, std::placeholders::_1));
}

void Server::HandleAccept(std::shared_ptr<ServerConnection> connection, const asio::error_code& error)
{
    if (!error)
    {
        connection->Start();
    }
    StartAccept();
}

}
