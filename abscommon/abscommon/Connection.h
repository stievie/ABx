#pragma once

#include "NetworkMessage.h"
#include <unordered_set>
#include <list>
#include <memory>
#include <asio.hpp>
#include <sa/SharedPtr.h>

namespace Net {

class ServicePort;
class OutputMessage;
class Protocol;
class Connection;

class ConnectionManager
{
public:
    static uint32_t maxPacketsPerSec;

    ConnectionManager() = default;
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    std::shared_ptr<Connection> CreateConnection(asio::io_service& ioService, std::shared_ptr<ServicePort> servicer);
    void ReleaseConnection(std::shared_ptr<Connection> connection);
    void CloseAll();
private:
    std::unordered_set<std::shared_ptr<Connection>> connections_;
    std::mutex lock_;
};

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    enum { WriteTimeout = 30 };
    enum { ReadTimeout = 30 };
    enum class State
    {
        Open = 0,
        Closed = 3
    };
public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(asio::io_service& ioService, std::shared_ptr<ServicePort> servicPort) :
        socket_(ioService),
        servicePort_(servicPort),
        readTimer_(asio::steady_timer(ioService)),
        writeTimer_(asio::steady_timer(ioService)),
        msg_(NetworkMessage::GetNew())
    {
        state_ = State::Open;
        receivedFirst_ = false;
        packetsSent_ = 0;
        timeConnected_ = time(nullptr);
    }
    ~Connection();

    /// Send the message
    bool Send(sa::SharedPtr<OutputMessage> message);
    /// Close the connection
    void Close(bool force = false);
    /// Used by protocols that require server to send first
    void Accept(std::shared_ptr<Protocol> protocol);
    void Accept();
    asio::ip::tcp::socket& GetSocket() { return socket_; }
    uint32_t GetIP() const;
    uint16_t GetPort() const;
protected:
    asio::ip::tcp::socket socket_;
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
    int64_t lastReadHeader_;
    int64_t lastReadBody_;
#endif
    std::shared_ptr<ServicePort> servicePort_;
    std::shared_ptr<Protocol> protocol_;
    std::mutex lock_;
    bool receivedFirst_;
    asio::steady_timer readTimer_;
    asio::steady_timer writeTimer_;
    std::unique_ptr<NetworkMessage> msg_;
    std::list<sa::SharedPtr<OutputMessage>> messageQueue_;
    time_t timeConnected_;
    uint32_t packetsSent_;

    std::atomic<State> state_;
};

}
