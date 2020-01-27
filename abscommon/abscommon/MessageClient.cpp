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
#include "MessageClient.h"

namespace Net {

void MessageClient::Connect(const std::string& host, uint16_t port, MessageHandler&& messageHandler)
{
    host_ = host;
    port_ = port;
    messageHandler_ = std::move(messageHandler);
    InternalConnect();
}

void MessageClient::InternalConnect()
{
    if (connected_)
        return;

    const asio::ip::tcp::resolver::query query(asio::ip::tcp::v4(), host_, std::to_string(port_));
    asio::ip::tcp::resolver::iterator endpoint = resolver_.resolve(query);
    asio::error_code error;
    socket_.connect(*endpoint, error);
    if (!error)
    {
        connected_ = true;
        AsyncReceive(&readMsg_,
            std::bind(&MessageClient::HandleRead,
                this, std::placeholders::_1, std::placeholders::_2));
    }
    else
        LOG_ERROR << "(" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
}

void MessageClient::HandleRead(const asio::error_code& error, size_t)
{
    if (!connected_)
        return;

    if (!error)
    {
        if (messageHandler_ && !readMsg_.IsEmpty())
            messageHandler_(readMsg_);
        readMsg_.Empty();
        AsyncReceive(&readMsg_,
            std::bind(&MessageClient::HandleRead,
                this, std::placeholders::_1, std::placeholders::_2));
    }
    else
    {
        LOG_ERROR << "(" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        Close();
    }
}

void MessageClient::DoWrite(MessageMsg msg)
{
    bool write_in_progress = !writeMsgs_.empty();
    writeMsgs_.push_back(msg);
    if (!write_in_progress)
    {
        asio::async_write(socket_,
            asio::buffer(writeMsgs_.front().Data(),
                writeMsgs_.front().Length()),
            std::bind(&MessageClient::HandleWrite, this,
                std::placeholders::_1));
    }
}

void MessageClient::HandleWrite(const asio::error_code& error)
{
    if (!connected_)
        return;

    if (!error)
    {
        writeMsgs_.pop_front();
        if (!writeMsgs_.empty())
        {
            asio::async_write(socket_,
                asio::buffer(writeMsgs_.front().Data(),
                    writeMsgs_.front().Length()),
                std::bind(&MessageClient::HandleWrite, this,
                    std::placeholders::_1));
        }
    }
    else
    {
        LOG_ERROR << "(" << error.default_error_condition().value() << ") " << error.default_error_condition().message() << std::endl;
        Close();
    }
}

}
