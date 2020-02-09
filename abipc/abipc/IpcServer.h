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

#include <stdint.h>
#include <set>
#include "MessageHandlers.h"
#include <asio.hpp>
#include <AB/IPC/Message.h>

namespace IPC {

class ServerConnection;
class MessageBuffer;

class Server
{
private:
    asio::io_service& ioService_;
    asio::ip::tcp::acceptor acceptor_;
    std::set<std::shared_ptr<ServerConnection>> clients_;
    void StartAccept();
    void HandleAccept(std::shared_ptr<ServerConnection> connection, const asio::error_code& error);
    // Send a message to all clients
    void InternalSend(const MessageBuffer& msg);
public:
    Server(asio::io_service& ioService, const asio::ip::tcp::endpoint& endpoint);
    ~Server() = default;
    void RemoveConnection(std::shared_ptr<ServerConnection> conn);
    void AddConnection(std::shared_ptr<ServerConnection> conn);
    void HandleMessage(const MessageBuffer& msg);
    template <typename _Msg>
    void Send(_Msg& msg)
    {
        MessageBuffer buff;
        buff.type_ = _Msg::message_type;
        Add(msg, buff);
        buff.EncodeHeader();
        InternalSend(buff);
    }
    MessageHandlers handlers_;
};

}
