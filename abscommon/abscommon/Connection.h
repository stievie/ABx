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

#include <unordered_set>
#include <list>
#include <memory>
#include <asio.hpp>
#include <sa/SmartPtr.h>
#include "NetworkMessage.h"
#include <sa/Noncopyable.h>

namespace Net {

class ServicePort;
class OutputMessage;
class Protocol;
class Connection;

class ConnectionManager
{
    NON_COPYABLE(ConnectionManager)
    NON_MOVEABLE(ConnectionManager)
public:
    static uint32_t maxPacketsPerSec;

    ConnectionManager() = default;

    std::shared_ptr<Connection> CreateConnection(asio::io_service& ioService, std::shared_ptr<ServicePort> servicer);
    void ReleaseConnection(std::shared_ptr<Connection> connection);
    void CloseAll();
private:
    std::unordered_set<std::shared_ptr<Connection>> connections_;
    std::mutex lock_;
};

class Connection : public std::enable_shared_from_this<Connection>
{
    NON_COPYABLE(Connection)
    NON_MOVEABLE(Connection)
public:
    enum { WriteTimeout = 30 };
    enum { ReadTimeout = 30 };
    enum class State
    {
        Open = 0,
        Closed = 3
    };
public:
    Connection(asio::io_service& ioService, std::shared_ptr<ServicePort> servicPort);
    ~Connection();

    asio::ip::tcp::socket socket_;
    /// Send the message
    bool Send(sa::SharedPtr<OutputMessage>&& message);
    /// Close the connection
    void Close(bool force = false);
    /// Used by protocols that require server to send first
    void Accept(std::shared_ptr<Protocol> protocol);
    void Accept();
    asio::ip::tcp::socket& GetSocket() { return socket_; }
    uint32_t GetIP() const;
    uint16_t GetPort() const;
private:
    friend class ConnectionManager;

    static void HandleReadTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error);
    static void HandleWriteTimeout(std::weak_ptr<Connection> weakConn, const asio::error_code& error);

    void ParseHeader(const asio::error_code& error);
    void ParsePacket(const asio::error_code& error);
    void CloseSocket();
    void OnWriteOperation(const asio::error_code& error);
    void InternalSend(OutputMessage& message);

#ifdef DEBUG_NET
    int64_t lastReadHeader_{ 0 };
    int64_t lastReadBody_{ 0 };
#endif
    std::shared_ptr<ServicePort> servicePort_;
    std::shared_ptr<Protocol> protocol_;
    std::mutex lock_;
    bool receivedFirst_;
    asio::steady_timer readTimer_;
    asio::steady_timer writeTimer_;
    /// Message read from the client
    std::unique_ptr<NetworkMessage> msg_;
    /// Messages will be sent to the client
    std::list<sa::SharedPtr<OutputMessage>> messageQueue_;
    time_t timeConnected_;
    uint32_t packetsSent_;

    std::atomic<State> state_;
};

}
