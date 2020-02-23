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
#include "IpcServer.h"
#include "ServerConnection.h"
#include "MessageBuffer.h"
#include <algorithm>

namespace IPC {

Server::Server(asio::io_service& ioService, const asio::ip::tcp::endpoint& endpoint) :
    ioService_(ioService),
    acceptor_(ioService, endpoint)
{
    StartAccept();
}

void Server::RemoveConnection(std::shared_ptr<ServerConnection> conn)
{
    std::lock_guard<std::mutex> lock(lock_);
    if (onClientDisconnect)
        onClientDisconnect(*conn);

    clients_.erase(conn);
}

void Server::AddConnection(std::shared_ptr<ServerConnection> conn)
{
    std::lock_guard<std::mutex> lock(lock_);
    clients_.emplace(conn);

    if (onClientConnect)
        onClientConnect(*conn);
}

ServerConnection* Server::GetConnection(uint32_t id)
{
    auto it = std::find_if(clients_.begin(), clients_.end(), [&id](const std::shared_ptr<ServerConnection>& current)
    {
        return id == current->GetId();
    });
    if (it == clients_.end())
        return nullptr;

    return it->get();
}

void Server::DisconnectClient(ServerConnection& client)
{
    client.Close();
}

void Server::DisconnectClient(uint32_t id)
{
    auto* client = GetConnection(id);
    if (!client)
        return;
    DisconnectClient(*client);
}

void Server::InternalSend(const MessageBuffer& msg)
{
    for (auto& c : clients_)
        c->Send(msg);
}

void Server::InternalSendTo(ServerConnection& client, const MessageBuffer& msg)
{
    client.Send(msg);
}

void Server::HandleMessage(ServerConnection& client, MessageBuffer& msg)
{
    if (handlers_.Exists(msg.type_))
        handlers_.Call(msg.type_, client, msg);
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
        bool accept = true;
        if (onAccept)
        {
            if (!onAccept(*connection))
                accept = false;
        }
        if (accept)
        {
            AddConnection(connection);
            connection->Start();
        }
    }
    StartAccept();
}

}
