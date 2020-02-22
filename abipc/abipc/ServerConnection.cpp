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
#include "ServerConnection.h"
#include "IpcServer.h"
#include <iostream>

namespace IPC {

sa::IdGenerator<uint32_t> ServerConnection::sIdGen;

ServerConnection::ServerConnection(asio::io_service& ioService, Server& server) :
    socket_(ioService),
    strand_(ioService),
    server_(server),
    id_(sIdGen.Next())
{ }

void ServerConnection::Start()
{
    asio::async_read(socket_,
        asio::buffer(readBuffer_.Data(), MessageBuffer::HeaderLength),
        std::bind(
            &ServerConnection::HandleRead, shared_from_this(), std::placeholders::_1
        ));
}

void ServerConnection::Close()
{
    socket_.close();
    server_.RemoveConnection(shared_from_this());
}

void ServerConnection::HandleRead(const asio::error_code& error)
{
    if (error || !readBuffer_.DecodeHeader())
    {
        Close();
        return;
    }

    asio::read(socket_, asio::buffer(readBuffer_.Body(), readBuffer_.BodyLength()));

    // Client sent a message
    server_.HandleMessage(*this, readBuffer_);

    readBuffer_.Empty();
    asio::async_read(socket_,
        asio::buffer(readBuffer_.Data(), MessageBuffer::HeaderLength),
        std::bind(
            &ServerConnection::HandleRead, shared_from_this(), std::placeholders::_1
        ));
}

void ServerConnection::HandleWrite(const asio::error_code& error, size_t)
{
    writeBuffers_.pop_front();
    if (error)
        return;
    if (!writeBuffers_.empty())
        Write();
}

void ServerConnection::WriteImpl(const MessageBuffer& msg)
{
    writeBuffers_.push_back(msg);
    if (writeBuffers_.size() > 1)
        // Write in progress
        return;
    Write();
}

void ServerConnection::Write()
{
    const MessageBuffer& msg = writeBuffers_[0];
    asio::async_write(
        socket_,
        asio::buffer(msg.Data(), msg.Length()),
        strand_.wrap(
            std::bind(
                &ServerConnection::HandleWrite,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2
            )
        )
    );
}

void ServerConnection::Send(const MessageBuffer& msg)
{
    strand_.post(std::bind(&ServerConnection::WriteImpl, this, msg));
}

uint32_t ServerConnection::GetIP() const
{
    asio::error_code err;
    const asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(err);
    if (!err)
        return endpoint.address().to_v4().to_uint();
    return 0;
}

uint16_t ServerConnection::GetPort() const
{
    asio::error_code err;
    const asio::ip::tcp::endpoint endpoint = socket_.remote_endpoint(err);
    if (!err)
        return endpoint.port();
    return 0;
}

}
