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

#pragma once

#include <functional>
#include <asio.hpp>
#include "MessageMsg.h"
#include "Logger.h"

namespace Net {

/// Client to send and receive messages from/to the Message Server.
class MessageClient
{
public:
    typedef std::function<void(const asio::error_code& error, size_t bytes_transferred)> ReadHandler;
    typedef std::function<void(const MessageMsg& msg)> MessageHandler;
private:
    asio::io_service& ioService_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket socket_;
    bool connected_{ false };
    std::string host_;
    uint16_t port_{ 0 };
    MessageMsg readMsg_;
    MessageQueue writeMsgs_;
    MessageHandler messageHandler_;
    void HandleReadHeader(MessageMsg* msg, const ReadHandler& handler,
        const asio::error_code& error, size_t bytes_transferred)
    {
        if (!error && msg->DecodeHeader())
        {
            asio::async_read(socket_,
                asio::buffer(msg->Body(), msg->BodyLength()),
                handler);
        }
        else
        {
            handler(error, bytes_transferred);
        }
    }
    void AsyncReceive(MessageMsg* msg, const ReadHandler& handler)
    {
        asio::async_read(socket_,
            asio::buffer(msg->Data(), MessageMsg::HeaderLength),
            std::bind(
                &MessageClient::HandleReadHeader,
                this, msg, handler, std::placeholders::_1, std::placeholders::_2));
    }
    void InternalConnect();
public:
    void HandleRead(const asio::error_code& error, size_t);
    void DoWrite(MessageMsg msg);
    void HandleWrite(const asio::error_code& error);
public:
    MessageClient(asio::io_service& io_service) :
        ioService_(io_service),
        resolver_(io_service),
        socket_(io_service)
    { }
    ~MessageClient() = default;

    void Connect(const std::string& host, uint16_t port, MessageHandler&& messageHandler);
    bool Write(const MessageMsg& msg)
    {
        if (connected_)
        {
            ioService_.post(std::bind(&MessageClient::DoWrite, this, msg));
            return true;
        }
        LOG_WARNING << "Writing message while not connected" << std::endl;
        return false;
    }
    void Close()
    {
        connected_ = false;
        socket_.close();
    }
    bool IsConnected() const
    {
        return connected_;
    }
    const std::string& GetHost() const
    {
        return host_;
    }
    uint16_t GetPort() const
    {
        return port_;
    }
};

}
