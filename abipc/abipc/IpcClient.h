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

#include "Message.h"
#include "MessageBuffer.h"
#include <asio.hpp>
#include <functional>
#include <sa/CallableTable.h>
#include <sa/StringHash.h>
#include <sa/TypeName.h>

namespace IPC {

class ClientMessageHandlers
{
private:
    sa::CallableTable<size_t, void, const MessageBuffer&> handlers_;
public:
    template<typename _Msg>
    void Add(std::function<void(const _Msg& msg)>&& func)
    {
        static constexpr size_t message_type = sa::StringHash(sa::TypeName<_Msg>::Get());
        handlers_.Add(message_type, [handler = std::move(func)](const MessageBuffer& buffer)
        {
            static constexpr size_t message_type = sa::StringHash(sa::TypeName<_Msg>::Get());
            if (message_type != buffer.type_)
                return;
            // See IpcServer.h why const_cast
            auto packet = Get<_Msg>(const_cast<MessageBuffer&>(buffer));
            handler(packet);
        });
    }
    bool Exists(size_t type) const { return handlers_.Exists(type); }
    void Call(size_t type, const MessageBuffer& buffer)
    {
        handlers_.Call(type, buffer);
    }
};

class Client
{
public:
    typedef std::function<void(const asio::error_code& error, size_t bytes_transferred)> ReadHandler;
private:
    asio::io_service& ioService_;
    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket socket_;
    bool connected_{ false };
    std::string host_;
    uint16_t port_{ 0 };
    MessageBuffer readBuffer_;
    BufferQueue writeBuffers_;
    void InternalConnect();
    void HandleReadHeader(MessageBuffer* msg, const ReadHandler& handler,
        const asio::error_code& error, size_t bytes_transferred);
    void AsyncReceive(MessageBuffer* msg, const ReadHandler& handler);
    void HandleRead(const asio::error_code& error, size_t);
    void DoWrite(MessageBuffer msg);
    void HandleWrite(const asio::error_code& error);
    void HandleMessage(const MessageBuffer& msg);
    bool InternalSend(const MessageBuffer& msg);
public:
    Client(asio::io_service& io_service) :
        ioService_(io_service),
        resolver_(io_service),
        socket_(io_service)
    { }
    bool Connect(const std::string& host, uint16_t port);
    void Close()
    {
        connected_ = false;
        socket_.close();
    }
    bool IsConnected() const
    {
        return connected_;
    }
    template <typename _Msg>
    void Send(_Msg& msg)
    {
        static constexpr size_t message_type = sa::StringHash(sa::TypeName<_Msg>::Get());
        MessageBuffer buff;
        buff.type_ = message_type;
        Add(msg, buff);
        buff.EncodeHeader();
        InternalSend(buff);
    }
    ClientMessageHandlers handlers_;
};

}
